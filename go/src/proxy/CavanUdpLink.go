package proxy

import (
	"fmt"
	"net"
	"time"

	"../common"
)

const (
	WR_WIN_SIZE uint8 = 32
	RD_WIN_SIZE uint8 = WR_WIN_SIZE * 2
)

type ICavanUdpLink interface {
	OnPackReceived(link *CavanUdpLink, pack *CavanUdpPack)
	OnSendFailed(link *CavanUdpLink, command *CavanUdpCmdNode)
	OnKeepAlive(link *CavanUdpLink)
}

type CavanUdpCallback struct {
}

type CavanUdpLink struct {
	Callback  ICavanUdpLink
	Sock      *CavanUdpSock
	Addr      *net.UDPAddr
	ProxyConn net.Conn

	ProcessChan chan *CavanUdpPack
	CommandChan chan *CavanUdpCmdNode
	ReadChan    chan *CavanUdpPack
	WaitChan    chan *CavanUdpWaiter
	ExitChan    chan bool

	WriteWin [WR_WIN_SIZE]*CavanUdpCmdNode
	ReadWin  [RD_WIN_SIZE]*CavanUdpPack

	WriteHead *CavanUdpCmdNode
	WriteTail *CavanUdpCmdNode
	WaitHead  *CavanUdpWaiter
	WaitTail  *CavanUdpWaiter

	WriteIndex uint8
	ReadIndex  uint8

	LocalPort  uint16
	RemotePort uint16
	Overtime   time.Duration
}

func NewCavanUdpLink(sock *CavanUdpSock, addr *net.UDPAddr, port uint16, callback ICavanUdpLink) *CavanUdpLink {
	link := &CavanUdpLink{Sock: sock, Addr: addr, LocalPort: port, Callback: callback}
	link.ProcessChan = make(chan *CavanUdpPack, 100)
	link.CommandChan = make(chan *CavanUdpCmdNode)
	link.WaitChan = make(chan *CavanUdpWaiter)
	link.ReadChan = make(chan *CavanUdpPack)
	link.ExitChan = make(chan bool)
	link.Overtime = time.Millisecond * 200
	sock.Links[port] = link
	go link.ProcessLoop()
	go link.WriteLoop()
	return link
}

func (link *CavanUdpLink) NewWaiter(op CavanUdpOpCode) *CavanUdpWaiter {
	waiter := NewCavanUdpWaiter(op)
	link.WaitChan <- waiter
	return waiter
}

func (link *CavanUdpLink) ProxyLoop(conn net.Conn) {
	defer link.Close()

	link.ProxyConn = conn

	bytes := make([]byte, 542)

	for true {
		conn.SetReadDeadline(time.Now().Add(time.Minute * 5))

		length, err := conn.Read(bytes)
		if err != nil {
			fmt.Println(err)
			break
		}

		link.SendDataAsync(bytes[0:length])
	}
}

func (link *CavanUdpLink) StartProxyDaemon(url string, port int) *CavanUdpLink {
	addr, err := net.ResolveTCPAddr("tcp", url)
	if err != nil {
		fmt.Println(err)
		return nil
	}

	tcp, err := net.DialTCP("tcp", nil, addr)
	if err != nil {
		fmt.Println(err)
		return nil
	}

	udp := link.Sock.NewLink(link.Addr)
	if udp == nil {
		tcp.Close()
		return nil
	}

	udp.RemotePort = uint16(port)

	go udp.ProxyLoop(tcp)

	return udp
}

func (link *CavanUdpLink) ProcessPack(pack *CavanUdpPack) {
	op := pack.OpCode()

	if (op & 0x80) != 0 {
		var prev *CavanUdpWaiter

		for waiter := link.WaitHead; waiter != nil; waiter = waiter.Next {
			if waiter.OpCode == op {
				waiter.SetReady(pack)

				if prev == nil {
					link.WaitHead = waiter.Next
				} else {
					prev.Next = waiter.Next
				}

				if link.WaitTail == waiter {
					link.WaitTail = prev
				}

				break
			}

			prev = waiter
		}
	} else {
		switch op {
		case CavanUdpOpPing:
			fmt.Println("CavanUdpOpPing")

		case CavanUdpOpConn:
			body := pack.Body()
			port := common.DecodeValue16(body, 0)
			url := string(body[2:])
			udp := link.StartProxyDaemon(url, port)

			builder := NewCavanUdpCmdBuilder(0x80|CavanUdpOpConn, 2)
			if udp != nil {
				builder.AppendValue16(udp.LocalPort)
			}

			builder.Build(link).SendAsync()

		case CavanUdpOpData:
			conn := link.ProxyConn
			if conn != nil {
				common.CavanConnWriteAll(conn, pack.Body())
			}
		}
	}
}

func (link *CavanUdpLink) ProcessLoop() {
	for true {
		select {
		case pack := <-link.ProcessChan:
			link.ProcessPack(pack)

		case <-time.After(time.Second * 10):
			link.Callback.OnKeepAlive(link)

		case <-link.ExitChan:
			return
		}
	}
}

func (link *CavanUdpLink) SendCommandRaw(command *CavanUdpCmdNode) {
	if command.Callback.Prepare(link, command.Times) {
		link.Sock.CommandChan <- command
		command.Time = time.Now()
		command.Next = nil
		command.Times++

		if command.Times > 1 {
			link.Overtime += time.Microsecond * 100
		}

		if link.WriteHead == nil {
			link.WriteHead = command
		} else {
			link.WriteTail.Next = command
		}

		link.WriteTail = command
	} else {
		link.Callback.OnSendFailed(link, command)
		command.SetReady(false)
	}
}

func (link *CavanUdpLink) WriteLoop() {
	defer link.Close()

	for true {
		var write_ch <-chan *CavanUdpCmdNode
		var timer_ch <-chan time.Time

		command := link.WriteHead

		if command == nil {
			timer_ch = nil
		} else if command.Pending {
			delay := link.Overtime - time.Now().Sub(command.Time)
			if delay > time.Millisecond*100 {
				timer_ch = time.After(delay)
			} else {
				link.WriteHead = command.Next
				link.SendCommandRaw(command)
				continue
			}
		} else {
			link.WriteHead = command.Next
			continue
		}

		index := link.WriteIndex % WR_WIN_SIZE
		command = link.WriteWin[index]

		if command != nil && command.Pending {
			write_ch = nil
		} else {
			write_ch = link.CommandChan
		}

		select {
		case waiter := <-link.WaitChan:
			if link.WaitHead == nil {
				link.WaitHead = waiter
			} else {
				link.WaitTail.Next = waiter
			}

			waiter.Next = nil
			link.WaitTail = waiter

		case command := <-write_ch:
			command.Callback.Setup(link.WriteIndex)
			command.Pending = true
			link.WriteWin[index] = command
			link.WriteIndex++
			link.SendCommandRaw(command)

		case node := <-link.ReadChan:
			link.Callback.OnPackReceived(link, node)

		case <-timer_ch:
			break

		case <-link.ExitChan:
			return
		}
	}
}

func (link *CavanUdpLink) SendDataAsync(bytes []byte) *CavanUdpCmdNode {
	builder := NewCavanUdpCmdBuilder(CavanUdpOpData, len(bytes))
	builder.AppendBytes(bytes)

	command := builder.Build(link)
	command.SendAsync()

	return command
}

func (link *CavanUdpLink) SendDataSync(bytes []byte) bool {
	return link.SendDataAsync(bytes).WaitReady()
}

func (link *CavanUdpLink) SendPingAsync() *CavanUdpCmdNode {
	builder := NewCavanUdpCmdBuilder(CavanUdpOpPing, 0)
	command := builder.Build(link)
	command.SendAsync()
	return command
}

func (link *CavanUdpLink) SendPingSync() bool {
	return link.SendPingAsync().WaitReady()
}

func (link *CavanUdpLink) SetRemoteAddr(url string) error {
	addr, err := net.ResolveUDPAddr("udp", url)
	if err != nil {
		return err
	}

	link.Addr = addr

	return nil
}

func (link *CavanUdpLink) Close() {
	link.Addr = nil

	if link.Sock.FreeLink(link) {
		close(link.ExitChan)
	}

	conn := link.ProxyConn
	link.ProxyConn = nil

	if conn != nil {
		conn.Close()
	}
}

func (callback *CavanUdpCallback) OnPackReceived(link *CavanUdpLink, pack *CavanUdpPack) {
	switch pack.OpCode() {
	case CavanUdpOpAck:
		command := link.WriteWin[pack.Index()%WR_WIN_SIZE]
		if command != nil && command.Callback.MatchAck(pack) {
			command.SetReady(true)

			if command.Times == 1 {
				delay := time.Now().Sub(command.Time) * 2
				link.Overtime = (link.Overtime*7 + delay) / 8
			}
		}

	case CavanUdpOpErr:
		link.Close()

	default:
		builder := NewCavanUdpCmdBuilder(CavanUdpOpAck, 0)
		builder.BuildResponse(link, pack).SendToSock()

		index := pack.Index()

		if index == link.ReadIndex {
			for true {
				link.ProcessChan <- pack
				link.ReadIndex++

				index = link.ReadIndex % RD_WIN_SIZE
				pack = link.ReadWin[index]

				if pack == nil {
					break
				}

				link.ReadWin[index] = nil
			}
		} else {
			tail := link.ReadIndex + RD_WIN_SIZE - 1

			if link.ReadIndex < tail {
				if index < link.ReadIndex || index > tail {
					return
				}
			} else if index > tail && index < link.ReadIndex {
				return
			}

			link.ReadWin[index%RD_WIN_SIZE] = pack
		}
	}
}

func (callback *CavanUdpCallback) OnSendFailed(link *CavanUdpLink, command *CavanUdpCmdNode) {
	fmt.Println("CavanUdpCallback::OnSendFailed")
	link.Close()
}

func (callback *CavanUdpCallback) OnKeepAlive(link *CavanUdpLink) {
	link.SendPingAsync()
}
