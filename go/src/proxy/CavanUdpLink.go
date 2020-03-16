package proxy

import (
	"fmt"
	"net"
	"time"

	"../common"
)

const (
	WR_WIN_SIZE uint8 = 8
	RD_WIN_SIZE uint8 = WR_WIN_SIZE * 2
)

type ICavanUdpLink interface {
	OnPackReceived(link *CavanUdpLink, pack *CavanUdpPack)
}

type CavanUdpCallback struct {
}

type CavanUdpLink struct {
	Callback    ICavanUdpLink
	Sock        *CavanUdpSock
	Addr        *net.UDPAddr
	ProxyConn   net.Conn
	ProcessChan chan *CavanUdpPack
	CommandChan chan *CavanUdpCmdNode
	ReadChan    chan *CavanUdpPack
	WaitChan    chan *CavanUdpWaiter
	WriteWin    [WR_WIN_SIZE]*CavanUdpCmdNode
	ReadWin     [RD_WIN_SIZE]*CavanUdpPack
	WriteHead   *CavanUdpCmdNode
	WriteTail   *CavanUdpCmdNode
	WaitHead    *CavanUdpWaiter
	WaitTail    *CavanUdpWaiter
	WriteIndex  uint8
	ReadIndex   uint8
	LocalPort   uint16
	RemotePort  uint16
	RTT         time.Duration
}

func NewCavanUdpLink(sock *CavanUdpSock, addr *net.UDPAddr, port uint16, callback ICavanUdpLink) *CavanUdpLink {
	link := &CavanUdpLink{Sock: sock, Addr: addr, LocalPort: port, Callback: callback}
	link.ProcessChan = make(chan *CavanUdpPack, 100)
	link.CommandChan = make(chan *CavanUdpCmdNode)
	link.WaitChan = make(chan *CavanUdpWaiter)
	link.ReadChan = make(chan *CavanUdpPack)
	link.RTT = time.Millisecond * 200
	sock.Links[port] = link
	go link.ProcessLoop()
	go link.WriteLoop()
	return link
}

func (link *CavanUdpLink) NewWaiter(id CavanUdpPackType) *CavanUdpWaiter {
	waiter := NewCavanUdpWaiter(id)
	link.WaitChan <- waiter
	return waiter
}

func (link *CavanUdpLink) ProxyLoop(tcp *net.TCPConn) {
	defer link.Close()
	defer tcp.Close()

	bytes := make([]byte, 1024)

	for true {
		tcp.SetReadDeadline(time.Now().Add(time.Minute * 5))

		length, err := tcp.Read(bytes)
		if err != nil {
			fmt.Println(err)
			break
		}

		if link.SendData(bytes[0:length]) == false {
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
	udp.ProxyConn = tcp

	go udp.ProxyLoop(tcp)

	return udp
}

func (link *CavanUdpLink) ProcessLoop() {
	for pack := range link.ProcessChan {
		code := pack.Type()

		if (code & 0x80) != 0 {
			var prev *CavanUdpWaiter

			for waiter := link.WaitHead; waiter != nil; waiter = waiter.Next {
				if waiter.PackType == code {
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
			switch pack.Type() {
			case CavanUdpPackConn:
				fmt.Println("CavanUdpPackConn")

				body := pack.Body()
				port := common.DecodeValue16(body, 0)
				url := string(body[2:])

				fmt.Println("port = ", port)
				fmt.Println("url = ", url)

				udp := link.StartProxyDaemon(url, port)

				builder := NewCavanUdpCmdBuilder(0x80|CavanUdpPackConn, 2)
				if udp != nil {
					builder.AppendValue16(udp.LocalPort)
				}

				builder.Build(link).SendAsync()

			case CavanUdpPackData:
				fmt.Println("CavanUdpPackData")

				conn := link.ProxyConn
				fmt.Println("conn = ", conn)

				if conn != nil {
					common.CavanConnWriteAll(conn, pack.Body())
				}
			}
		}
	}
}

func (link *CavanUdpLink) SendCommandRaw(command *CavanUdpCmdNode) {
	if command.Callback.Prepare(link, command.Times) {
		link.Sock.WriteChan <- command
		command.Time = time.Now().Add(link.RTT)
		command.Times++

		if link.WriteHead == nil {
			link.WriteHead = command
		} else {
			link.WriteTail.Next = command
		}

		link.WriteTail = command
	} else {
		command.SetReady(false)
	}
}

func (link *CavanUdpLink) WriteLoop() {
	for true {
		var write_ch <-chan *CavanUdpCmdNode
		var timer_ch <-chan time.Time

		command := link.WriteHead

		if command == nil {
			timer_ch = nil
		} else if command.Pending {
			delay := command.Time.Sub(time.Now())
			if delay > 0 {
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
			link.RTT++
		}
	}
}

func (link *CavanUdpLink) SendData(bytes []byte) bool {
	builder := NewCavanUdpCmdBuilder(CavanUdpPackData, len(bytes))
	builder.AppendBytes(bytes)
	return builder.Build(link).SendSync()
}

func (link *CavanUdpLink) SendPing() bool {
	builder := NewCavanUdpCmdBuilder(CavanUdpPackPing, 0)
	return builder.Build(link).SendSync()
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
	fmt.Println("CavanUdpLinkClose")
}

func (callback *CavanUdpCallback) OnPackReceived(link *CavanUdpLink, pack *CavanUdpPack) {
	fmt.Println("OnPackReceived: ", pack.Bytes)

	if pack.Type() == CavanUdpPackAck {
		fmt.Println("CavanUdpPackAck")
		index := int(pack.Index()) % len(link.WriteWin)
		command := link.WriteWin[index]
		command.SetReady(true)
	} else {
		builder := NewCavanUdpCmdBuilder(CavanUdpPackAck, 0)
		builder.BuildResponse(link, pack).SendToSock()

		link.ProcessChan <- pack
	}
}
