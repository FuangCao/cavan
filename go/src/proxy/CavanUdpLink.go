package proxy

import (
	"fmt"
	"net"
	"time"

	"../common"
)

type ICavanUdpLink interface {
	OnPackReceived(link *CavanUdpLink, pack *CavanUdpPack)
}

type CavanUdpCallback struct {
	Conn net.Conn
}

type CavanUdpLink struct {
	Callback   ICavanUdpLink
	Sock       *CavanUdpSock
	Addr       *net.UDPAddr
	WriteChan  chan *CavanUdpCmdNode
	ReadChan   chan *CavanUdpPack
	WriteWin   [8]*CavanUdpCmdNode
	WriteHead  *CavanUdpCmdNode
	WriteTail  *CavanUdpCmdNode
	ReadWin    [16]*CavanUdpPack
	WriteIndex uint8
	ReadIndex  uint8
	LocalPort  uint16
	RemotePort uint16
	RTT        time.Duration
}

func NewCavanUdpLink(sock *CavanUdpSock, addr *net.UDPAddr, port uint16, callback ICavanUdpLink) *CavanUdpLink {
	link := &CavanUdpLink{Sock: sock, Addr: addr, LocalPort: port, Callback: callback}
	link.WriteChan = make(chan *CavanUdpCmdNode)
	link.ReadChan = make(chan *CavanUdpPack)
	link.RTT = time.Millisecond * 200
	sock.Links[port] = link
	go link.WriteLoop()
	return link
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
		link.SetCommandReady(command, nil)
	}
}

func (link *CavanUdpLink) WriteLoop() {
	for true {
		var timer_ch <-chan time.Time
		var write_ch <-chan *CavanUdpCmdNode

		command := link.WriteHead
		if command != nil {
			delay := command.Time.Sub(time.Now())
			if delay > 0 {
				timer_ch = time.After(delay)
			} else {
				link.WriteHead = command.Next
				if link.WriteWin[command.Index] == command {
					link.SendCommandRaw(command)
				}
				continue
			}
		} else {
			timer_ch = nil
		}

		index := link.WriteIndex % 8
		if link.WriteWin[index] == nil {
			write_ch = link.WriteChan
		} else {
			write_ch = nil
		}

		select {
		case command := <-write_ch:
			command.Callback.Setup(link, link.WriteIndex)
			command.Index = int(link.WriteIndex) % 8
			command.Link = link
			link.WriteWin[command.Index] = command
			link.WriteIndex++
			link.SendCommandRaw(command)

		case node := <-link.ReadChan:
			link.Callback.OnPackReceived(link, node)

		case <-timer_ch:
			link.RTT++
		}
	}
}

func (link *CavanUdpLink) SendCommandAsync(command *CavanUdpCmdNode) {
	link.WriteChan <- command
}

func (link *CavanUdpLink) WaitCommandReady(command *CavanUdpCmdNode) *CavanUdpPack {
	return <-command.DoneChan
}

func (link *CavanUdpLink) SendCommandSync(command *CavanUdpCmdNode) *CavanUdpPack {
	link.SendCommandAsync(command)
	return link.WaitCommandReady(command)
}

func (link *CavanUdpLink) SetCommandReady(command *CavanUdpCmdNode, pack *CavanUdpPack) {
	if link.WriteWin[command.Index] == command {
		link.WriteWin[command.Index] = nil
		command.DoneChan <- pack
	}
}

func (link *CavanUdpLink) SendDataSync(bytes []byte) *CavanUdpPack {
	builder := NewCavanUdpCmdBuilder(CavanUdpCmdData, len(bytes))
	builder.AppendBytes(bytes)
	return link.SendCommandSync(builder.Build())
}

func (link *CavanUdpLink) SendPing() *CavanUdpPack {
	builder := NewCavanUdpCmdBuilder(CavanUdpCmdPing, 0)
	return link.SendCommandSync(builder.Build())
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

func (callback *CavanUdpCallback) StartProxyDaemon(link *CavanUdpLink, url string, port int) *CavanUdpLink {
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

	callback.Conn = tcp
	go callback.ProxyLoop(link, tcp, udp)

	return udp
}

func (callback *CavanUdpCallback) ProxyLoop(link *CavanUdpLink, tcp *net.TCPConn, udp *CavanUdpLink) {
	defer tcp.Close()
	defer udp.Close()

	bytes := make([]byte, 1024)

	for true {
		tcp.SetReadDeadline(time.Now().Add(time.Minute * 5))

		length, err := tcp.Read(bytes)
		if err != nil {
			fmt.Println(err)
			break
		}

		if link.SendDataSync(bytes[0:length]) == nil {
			break
		}
	}
}

func (callback *CavanUdpCallback) OnPackReceived(link *CavanUdpLink, pack *CavanUdpPack) {
	fmt.Println("OnPackReceived: ", pack.Bytes)

	if pack.Type() == CavanUdpRspSuccess {
		fmt.Println("CavanUdpRspSuccess")
		index := int(pack.Index()) % len(link.WriteWin)
		command := link.WriteWin[index]
		link.SetCommandReady(command, pack)
	} else {
		callback.ProcessUdpPack(link, pack)
	}
}

func (callback *CavanUdpCallback) ProcessUdpPack(link *CavanUdpLink, pack *CavanUdpPack) {
	switch pack.Type() {
	case CavanUdpCmdPing:
		fmt.Println("CavanUdpCmdPing")
		builder := NewCavanUdpCmdBuilder(CavanUdpRspSuccess, 0)
		link.Sock.WriteChan <- builder.BuildResponse(link, pack)

	case CavanUdpCmdConn:
		fmt.Println("CavanUdpCmdConn")
		body := pack.Body()
		port := common.DecodeValue16(body, 0)
		url := string(body[2:])

		fmt.Println("port = ", port)
		fmt.Println("url = ", url)

		udp := callback.StartProxyDaemon(link, url, port)
		builder := NewCavanUdpCmdBuilder(CavanUdpRspSuccess, 2)
		if udp != nil {
			builder.AppendValue16(udp.LocalPort)
		}

		link.Sock.WriteChan <- builder.BuildResponse(link, pack)

	case CavanUdpCmdData:
		fmt.Println("CavanUdpCmdData")
		builder := NewCavanUdpCmdBuilder(CavanUdpRspSuccess, 0)
		link.Sock.WriteChan <- builder.BuildResponse(link, pack)

		conn := callback.Conn
		fmt.Println("conn = ", conn)
		if conn != nil {
			common.CavanConnWriteAll(callback.Conn, pack.Body())
		}
	}
}
