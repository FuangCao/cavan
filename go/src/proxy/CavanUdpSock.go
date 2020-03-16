package proxy

import (
	"fmt"
	"net"
	"time"

	"../common"
)

type CavanUdpSock struct {
	Conn      *net.UDPConn
	Index     int
	Links     [1 << 16]*CavanUdpLink
	StunLink  *CavanUdpLink
	WriteChan chan *CavanUdpCmdNode
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
	sock.WriteChan = make(chan *CavanUdpCmdNode, 100)

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
	index := sock.NewPort()
	if index < 0 {
		return nil
	}

	return NewCavanUdpLink(sock, addr, uint16(index), &CavanUdpCallback{})
}

func (sock *CavanUdpSock) WriteLoop() {
	for pack := range sock.WriteChan {
		_, err := pack.Callback.WriteTo(pack.Link, sock.Conn)
		if err != nil {
			fmt.Println(err)
			time.Sleep(time.Minute)
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
				fmt.Println(addr)
			}
		}
	}
}

func (sock *CavanUdpSock) GetWanAddr() *net.UDPAddr {
	builder := NewCavanStunCmdBuilder(0)
	builder.SetType(1)

	command := builder.Build()
	response := sock.StunLink.SendCommandSync(command)
	if response == nil {
		return nil
	}

	pack := CavanStunPack{Bytes: response.Bytes}

	return pack.GetWanAddr()
}

func (callback *CavanStunCallback) OnPackReceived(link *CavanUdpLink, pack *CavanUdpPack) {
	command := link.WriteHead
	if command != nil {
		link.WriteHead = command.Next
		link.SetCommandReady(command, pack)
	}
}
