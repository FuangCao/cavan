package proxy

import (
	"net"
	"time"

	"../common"
)

type CavanUdpCmdCallback interface {
	WriteTo(link *CavanUdpLink, conn *net.UDPConn) (int, error)
	Prepare(link *CavanUdpLink, times int) bool
	Setup(index uint8)
	RspOpCode() CavanUdpOpCode
	MatchAck(pack *CavanUdpPack) bool
}

type CavanUdpCmdNode struct {
	Callback CavanUdpCmdCallback
	Next     *CavanUdpCmdNode
	Link     *CavanUdpLink
	Time     time.Time
	Times    int
	Pending  bool
	DoneChan chan bool
}

type CavanUdpCommand struct {
	CavanUdpPack
}

type CavanUdpCmdBuilder struct {
	common.ByteArrayBuilder
	CavanUdpPack
}

func NewCavanUdpCmdNode(link *CavanUdpLink, callback CavanUdpCmdCallback) *CavanUdpCmdNode {
	node := CavanUdpCmdNode{Link: link, Callback: callback}
	node.DoneChan = make(chan bool, 2)
	return &node
}

func (command *CavanUdpCmdNode) NewRspWaiter() *CavanUdpWaiter {
	pack := command.Callback.RspOpCode()
	return command.Link.NewWaiter(pack)
}

func (command *CavanUdpCmdNode) WaitReady() bool {
	return <-command.DoneChan
}

func (command *CavanUdpCmdNode) SendAsync() {
	link := command.Link
	if link.Closed {
		command.DoneChan <- false
	} else {
		link.CommandChan <- command
	}
}

func (command *CavanUdpCmdNode) SetReady(success bool) {
	if command.Pending {
		command.Pending = false
		command.DoneChan <- success
	}
}

func (command *CavanUdpCmdNode) SendSync() bool {
	command.SendAsync()
	return command.WaitReady()
}

func (command *CavanUdpCmdNode) SendToSock() {
	command.Link.Sock.CommandChan <- command
}

func (command *CavanUdpCmdNode) SendWaitResponse(delay time.Duration) *CavanUdpPack {
	waiter := command.NewRspWaiter()

	if command.SendSync() {
		return waiter.WaitReady(delay)
	}

	return nil
}

func (command *CavanUdpCommand) WriteTo(link *CavanUdpLink, conn *net.UDPConn) (int, error) {
	return conn.WriteToUDP(command.Bytes, link.Addr)
}

func (command *CavanUdpCommand) Prepare(link *CavanUdpLink, times int) bool {
	return times < 20
}

func (command *CavanUdpCommand) Setup(index uint8) {
	command.SetIndex(index)
}

func (command *CavanUdpCommand) RspOpCode() CavanUdpOpCode {
	return command.OpCode() | 0x80
}

func (command *CavanUdpCommand) MatchAck(pack *CavanUdpPack) bool {
	return command.Index() == pack.Index()
}

func NewCavanUdpCmdBuilder(op CavanUdpOpCode, length int) *CavanUdpCmdBuilder {
	bytes := make([]byte, length+6)
	builder := CavanUdpCmdBuilder{}
	builder.CavanUdpPack.Bytes = bytes
	builder.ByteArrayBuilder.Init(bytes, 6)
	builder.SetOpCode(op)
	return &builder
}

func (builder *CavanUdpCmdBuilder) Build(link *CavanUdpLink) *CavanUdpCmdNode {
	builder.SetDestPort(link.RemotePort)
	builder.SetSrcPort(link.LocalPort)
	command := CavanUdpCommand{}
	command.Bytes = builder.ByteArrayBuilder.Build()
	return NewCavanUdpCmdNode(link, &command)
}

func (builder *CavanUdpCmdBuilder) BuildResponse(link *CavanUdpLink, command *CavanUdpPack) *CavanUdpCmdNode {
	builder.SetIndex(command.Index())
	return builder.Build(link)
}