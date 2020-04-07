package proxy

import (
	"fmt"
	"net"
	"time"

	"../common"
)

const (
	WR_WIN_SIZE    uint8         = 64
	RD_WIN_SIZE    uint8         = WR_WIN_SIZE * 2
	SEND_DELAY_MAX time.Duration = time.Second * 3
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
	Closed    bool
	ProxyConn net.Conn
	RTT       time.Duration

	WriteTime  time.Time
	WriteDelay time.Duration

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
}

func NewCavanUdpLink(sock *CavanUdpSock, addr *net.UDPAddr, port uint16, callback ICavanUdpLink) *CavanUdpLink {
	link := &CavanUdpLink{Sock: sock, Addr: addr, LocalPort: port, Callback: callback}
	link.CommandChan = make(chan *CavanUdpCmdNode, 100)
	link.ProcessChan = make(chan *CavanUdpPack, 100)
	link.WaitChan = make(chan *CavanUdpWaiter, 10)
	link.ReadChan = make(chan *CavanUdpPack, 10)
	link.ExitChan = make(chan bool)
	link.RTT = time.Millisecond * 50
	sock.Links[port] = link
	go link.ProcessLoop()
	go link.WriteLoop()
	return link
}

func (link *CavanUdpLink) NewWaiter(op CavanUdpOpCode) *CavanUdpWaiter {
	waiter := NewCavanUdpWaiter(link, op)
	link.WaitChan <- waiter
	return waiter
}

func (link *CavanUdpLink) ProxyLoop(conn net.Conn) {
	defer link.Close(false)

	link.ProxyConn = conn

	bytes := make([]byte, 1466)

	for true {
		// conn.SetReadDeadline(time.Now().Add(time.Minute * 5))

		length, err := conn.Read(bytes)
		if err != nil {
			fmt.Println(err)
			break
		}

		if link.SendDataAsync(bytes[0:length]) == nil {
			break
		}
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

			fmt.Println("url =", url)
			fmt.Println("port =", port)

			udp := link.StartProxyDaemon(url, port)

			builder := NewCavanUdpCmdBuilder(0x80|CavanUdpOpConn, 2)
			if udp != nil {
				builder.AppendValue16(udp.LocalPort)
			}

			builder.Build(link).SendAsync()

		case CavanUdpOpData:
			// fmt.Println("CavanUdpOpData")
			conn := link.ProxyConn
			if conn != nil {
				common.CavanConnWriteAll(conn, pack.Body())
			}

		case CavanUdpOpClose:
			fmt.Println("CavanUdpOpClose")
			link.Close(true)
		}
	}
}

func (link *CavanUdpLink) ProcessLoop() {
	for true {
		select {
		case pack := <-link.ProcessChan:
			link.ProcessPack(pack)

		case <-time.After(time.Second * 20):
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

		link.WriteTime = command.Time

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
	defer func() {
		link.Close(true)

		for link.WriteHead != nil {
			link.WriteHead.SetReady(false)
			link.WriteHead = link.WriteHead.Next
		}

		for link.WaitHead != nil {
			link.WaitHead.SetReady(nil)
			link.WaitHead = link.WaitHead.Next
		}

		for true {
			select {
			case command := <-link.CommandChan:
				command.SetReady(false)

			case waiter := <-link.WaitChan:
				waiter.SetReady(nil)

			default:
				return
			}
		}
	}()

	link.WriteDelay = time.Millisecond * 100

	for true {
		var write_ch <-chan *CavanUdpCmdNode
		var timer_ch <-chan time.Time
		var delay0 time.Duration
		var delay1 time.Duration
		var index uint8

		for true {
			now := time.Now()
			// fmt.Println("now =", now)

			delay0 = link.WriteDelay - now.Sub(link.WriteTime)

			command := link.WriteHead
			if command == nil {
				delay1 = 0
				break
			}

			if command.Pending {
				delay1 = link.RTT*2 - now.Sub(command.Time)
				if delay0 > 0 || delay1 > 0 {
					break
				}

				if link.WriteDelay < 10 {
					if link.WriteDelay > 0 {
						link.WriteDelay++
					} else {
						link.WriteDelay = 1
					}
				} else if link.WriteDelay < time.Microsecond {
					link.WriteDelay += 100
				} else {
					link.WriteDelay *= 2

					if link.WriteDelay > SEND_DELAY_MAX {
						link.WriteDelay = SEND_DELAY_MAX
					}
				}

				link.WriteHead = command.Next
				link.RTT += time.Millisecond
				link.SendCommandRaw(command)
			} else {
				link.WriteHead = command.Next

				if link.WriteDelay < 10 {
					link.WriteDelay--
				} else if link.WriteDelay < time.Microsecond {
					link.WriteDelay -= 100
				} else {
					link.WriteDelay /= 2
				}
			}
		}

		index = link.WriteIndex % WR_WIN_SIZE

		// fmt.Println("delay0 =", delay0, ", delay1 =", delay1)

		if delay0 > 0 {
			if delay0 < delay1 {
				delay0 = delay1
			}

			timer_ch = time.After(delay0)
			write_ch = nil
		} else {
			if delay1 > 0 {
				timer_ch = time.After(delay1)
			} else {
				timer_ch = nil
			}

			command := link.WriteWin[index]
			if command != nil && command.Pending {
				write_ch = nil
			} else {
				write_ch = link.CommandChan
			}
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
	return builder.Build(link).SendAsync()
}

func (link *CavanUdpLink) SendDataSync(bytes []byte) bool {
	command := link.SendDataAsync(bytes)
	if command == nil {
		return false
	}

	return command.WaitReady()
}

func (link *CavanUdpLink) SendPingAsync() *CavanUdpCmdNode {
	builder := NewCavanUdpCmdBuilder(CavanUdpOpPing, 0)
	return builder.Build(link).SendAsync()
}

func (link *CavanUdpLink) SendPingSync() bool {
	command := link.SendPingAsync()
	if command == nil {
		return false
	}

	return command.WaitReady()
}

func (link *CavanUdpLink) SetRemoteAddr(url string) error {
	addr, err := net.ResolveUDPAddr("udp", url)
	if err != nil {
		return err
	}

	link.Addr = addr

	return nil
}

func (link CavanUdpLink) WaitCloseComplete(command *CavanUdpCmdNode) bool {
	if command == nil {
		return false
	}

	success := command.WaitReady()
	fmt.Println("WaitCloseComplete:", success)
	link.Close(true)

	return success
}

func (link *CavanUdpLink) Close(force bool) {
	if force {
		if link.Sock.FreeLink(link) {
			close(link.ExitChan)
		}
	} else {
		builder := NewCavanUdpCmdBuilder(CavanUdpOpClose, 0)
		go link.WaitCloseComplete(builder.Build(link).SendAsync())
	}

	link.Closed = true

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
				delay := time.Now().Sub(command.Time)
				link.RTT = (link.RTT*7 + delay) / 8
			}
		}

	case CavanUdpOpErr:
		link.Close(true)

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
	link.Close(true)
}

func (callback *CavanUdpCallback) OnKeepAlive(link *CavanUdpLink) {
	link.SendPingAsync()
}
