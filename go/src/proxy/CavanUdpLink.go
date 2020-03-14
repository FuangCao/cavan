package proxy

import (
	"fmt"
	"net"
	"time"
)

type ICavanUdpLink interface {
	OnPackReceived(link *CavanUdpLink, node *CavanUdpRspNode)
}

type CavanUdpCallback struct {
}

type CavanUdpLink struct {
	Callback   ICavanUdpLink
	Sock       *CavanUdpSock
	Addr       *net.UDPAddr
	WriteChan  chan *CavanUdpCmdNode
	ReadChan   chan *CavanUdpRspNode
	WriteWin   [8]*CavanUdpCmdNode
	WriteHead  *CavanUdpCmdNode
	WriteTail  *CavanUdpCmdNode
	WriteIndex int
	LocalPort  uint16
	RemotePort uint16
	RTT        time.Duration
}

func NewCavanUdpLink(sock *CavanUdpSock, addr *net.UDPAddr, port uint16, callback ICavanUdpLink) *CavanUdpLink {
	link := &CavanUdpLink{Sock: sock, Addr: addr, LocalPort: port, Callback: callback}
	link.WriteChan = make(chan *CavanUdpCmdNode)
	link.ReadChan = make(chan *CavanUdpRspNode)
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
		var timer <-chan time.Time

		command := link.WriteHead
		if command != nil {
			delay := command.Time.Sub(time.Now())
			if delay > 0 {
				timer = time.After(delay)
			} else {
				link.WriteHead = command.Next
				link.SendCommandRaw(command)
				continue
			}
		} else {
			timer = nil
		}

		select {
		case command := <-link.WriteChan:
			command.Callback.Setup(link, link.WriteIndex)
			command.Index = link.WriteIndex % 8
			link.WriteWin[command.Index] = command
			link.WriteIndex++
			link.SendCommandRaw(command)

		case node := <-link.ReadChan:
			link.Callback.OnPackReceived(link, node)

		case <-timer:
			link.RTT++
		}
	}
}

func (link *CavanUdpLink) SendCommandAsync(command *CavanUdpCmdNode) {
	link.WriteChan <- command
}

func (link *CavanUdpLink) WaitCommandReady(command *CavanUdpCmdNode) *CavanUdpRspNode {
	return <-command.DoneChan
}

func (link *CavanUdpLink) SendCommandSync(command *CavanUdpCmdNode) *CavanUdpRspNode {
	link.SendCommandAsync(command)
	return link.WaitCommandReady(command)
}

func (link *CavanUdpLink) SetCommandReady(command *CavanUdpCmdNode, pack *CavanUdpRspNode) {
	if link.WriteWin[command.Index] == command {
		link.WriteWin[command.Index] = nil
		command.DoneChan <- pack
	}
}

func (callback *CavanUdpCallback) OnPackReceived(link *CavanUdpLink, node *CavanUdpRspNode) {
	pack := CavanUdpPack{Bytes: node.Bytes}
	fmt.Println(pack)
}
