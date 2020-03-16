package proxy

import (
	"net"
	"time"

	"../common"
)

type CavanUdpCmdCallback interface {
	WriteTo(link *CavanUdpLink, conn *net.UDPConn) (int, error)
	Prepare(link *CavanUdpLink, times int) bool
	Setup(link *CavanUdpLink, index uint8)
}

type CavanUdpCmdNode struct {
	Callback CavanUdpCmdCallback
	Next     *CavanUdpCmdNode
	Link     *CavanUdpLink
	Time     time.Time
	Times    int
	Index    int
	DoneChan chan *CavanUdpPack
}

type CavanUdpCommand struct {
	CavanUdpPack
}

type CavanUdpCmdBuilder struct {
	common.ByteArrayBuilder
	CavanUdpPack
}

func NewCavanUdpCmdNode(callback CavanUdpCmdCallback) *CavanUdpCmdNode {
	node := CavanUdpCmdNode{Callback: callback}
	node.DoneChan = make(chan *CavanUdpPack)
	return &node
}

func (command *CavanUdpCommand) WriteTo(link *CavanUdpLink, conn *net.UDPConn) (int, error) {
	return conn.WriteToUDP(command.Bytes, link.Addr)
}

func (command *CavanUdpCommand) Prepare(link *CavanUdpLink, times int) bool {
	return times < 5
}

func (command *CavanUdpCommand) Setup(link *CavanUdpLink, index uint8) {
	command.SetDestPort(int(link.RemotePort))
	command.SetSrcPort(int(link.LocalPort))
	command.SetIndex(index)
}

func NewCavanUdpCmdBuilder(command CavanUdpPackType, length int) *CavanUdpCmdBuilder {
	bytes := make([]byte, length+6)
	builder := CavanUdpCmdBuilder{}
	builder.CavanUdpPack.Bytes = bytes
	builder.ByteArrayBuilder.Init(bytes, 6)
	builder.SetType(command)
	return &builder
}

func (builder *CavanUdpCmdBuilder) Build() *CavanUdpCmdNode {
	bytes := builder.ByteArrayBuilder.Build()
	command := CavanUdpCommand{}
	command.Bytes = bytes
	return NewCavanUdpCmdNode(&command)
}

func (builder *CavanUdpCmdBuilder) BuildResponse(link *CavanUdpLink, command *CavanUdpPack) *CavanUdpCmdNode {
	builder.SetDestPort(command.SrcPort())
	builder.SetSrcPort(command.DestPort())
	builder.SetIndex(command.Index())
	node := builder.Build()
	node.Link = link
	return node
}
