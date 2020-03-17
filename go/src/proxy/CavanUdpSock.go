package proxy

import (
	"fmt"
	"net"
	"sync"
	"time"

	"../common"
)

type CavanUdpSock struct {
	sync.Mutex
	Conn        *net.UDPConn
	Index       int
	Links       [1 << 16]*CavanUdpLink
	StunLink    *CavanUdpLink
	CommandChan chan *CavanUdpCmdNode
	PackChan    chan *CavanUdpDirectPack
	WanAddr     *net.UDPAddr
	WanTime     time.Time
}

type CavanStunCallback struct {
}

func NewCavanUdpSock() *CavanUdpSock {
	conn, err := net.ListenUDP("udp", nil)
	if err != nil {
		fmt.Println(err)
		return nil
	}

	sock := CavanUdpSock{Conn: conn}
	sock.CommandChan = make(chan *CavanUdpCmdNode, 100)
	sock.PackChan = make(chan *CavanUdpDirectPack, 100)

	sock.StunLink = NewCavanUdpLink(&sock, nil, 0x0101, &CavanStunCallback{})
	sock.Links[0x0101] = sock.StunLink

	go sock.WriteLoop()
	go sock.ReadLoop()

	return &sock
}

func (sock *CavanUdpSock) NewPort() int {
	for i := sock.Index + 1; i < len(sock.Links); i++ {
		if sock.Links[i] == nil {
			sock.Index = i
			return i
		}
	}

	for i := 0; i < sock.Index; i++ {
		if sock.Links[i] == nil {
			sock.Index = i
			return i
		}
	}

	return -1
}

func (sock *CavanUdpSock) NewLink(addr *net.UDPAddr) *CavanUdpLink {
	sock.Lock()
	defer sock.Unlock()

	index := sock.NewPort()
	if index < 0 {
		return nil
	}

	fmt.Println("NewLink:", index)

	return NewCavanUdpLink(sock, addr, uint16(index), &CavanUdpCallback{})
}

func (sock *CavanUdpSock) FreeLink(link *CavanUdpLink) bool {
	sock.Lock()
	defer sock.Unlock()

	port := link.LocalPort

	if sock.Links[port] != link {
		return false
	}

	fmt.Println("FreeLink:", port)
	sock.Links[port] = nil

	return true
}

func (sock *CavanUdpSock) WriteLoop() {
	for true {
		select {
		case pack := <-sock.CommandChan:
			_, err := pack.Callback.WriteTo(pack.Link, sock.Conn)
			if err != nil {
				fmt.Println(err)
				time.Sleep(time.Minute)
			}

		case pack := <-sock.PackChan:
			sock.Conn.WriteToUDP(pack.Bytes, pack.Addr)
		}
	}
}

func (sock *CavanUdpSock) ReadLoop() {
	buff := make([]byte, 2048)
	pack := CavanUdpPack{Bytes: buff}

	for true {
		for true {
			length, addr, err := sock.Conn.ReadFromUDP(buff)
			if err != nil {
				fmt.Println(err)
				break
			}

			if length < 6 {
				continue
			}

			dest := pack.DestPort()
			link := sock.Links[dest]

			if link != nil && link.LocalPort == uint16(dest) {
				bytes := common.Clone(buff, length)
				link.ReadChan <- &CavanUdpPack{Bytes: bytes}
			} else {
				if pack.OpCode() > CavanUdpOpErr {
					response := NewCavanUdpDirectPack(addr, make([]byte, 6))
					response.SetDestPort(pack.SrcPort())
					response.SetSrcPort(pack.DestPort())
					response.SetIndex(pack.Index())
					response.SetOpCode(CavanUdpOpErr)
					sock.PackChan <- response
				}
			}
		}
	}
}

func (sock *CavanUdpSock) GetWanAddr() *net.UDPAddr {
	if sock.WanAddr != nil {
		delay := time.Now().Sub(sock.WanTime)
		if delay < time.Minute {
			return sock.WanAddr
		}
	}

	builder := NewCavanStunCmdBuilder(0)
	builder.SetType(1)

	if response := builder.Build(sock.StunLink).SendWaitResponse(time.Millisecond * 500); response != nil {
		pack := CavanStunPack{Bytes: response.Bytes}
		addr := pack.GetWanAddr()
		if addr == nil {
			return nil
		}

		sock.WanTime = time.Now()
		sock.WanAddr = addr

		return addr
	}

	return nil

	/* addr := sock.Conn.LocalAddr().(*net.UDPAddr)

	if addr, err := net.ResolveUDPAddr("udp", fmt.Sprintf("127.0.0.1:%d", addr.Port)); err == nil {
		return addr
	}

	return nil */
}

func (callback *CavanStunCallback) OnPackReceived(link *CavanUdpLink, pack *CavanUdpPack) {
	command := link.WriteHead
	if command != nil {
		link.WriteHead = command.Next
		command.SetReady(true)
	}

	waiter := link.WaitHead
	if waiter != nil {
		link.WaitHead = waiter.Next
		waiter.SetReady(pack)
	}
}

func (callback *CavanStunCallback) OnSendFailed(link *CavanUdpLink, command *CavanUdpCmdNode) {
	fmt.Println("CavanStunCallback:OnSendFailed")
}

func (callback *CavanStunCallback) OnKeepAlive(link *CavanUdpLink) {

}
