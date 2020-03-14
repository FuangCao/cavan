package proxy

import (
	"net"
	"time"

	"../common"
)

type CavanUdpRspNode struct {
	Bytes []byte
	Addr  *net.UDPAddr
}

type CavanUdpCmdCallback interface {
	WriteTo(link *CavanUdpLink, conn *net.UDPConn) (int, error)
	Prepare(link *CavanUdpLink, times int) bool
	Setup(link *CavanUdpLink, index int)
}

type CavanUdpCmdNode struct {
	Callback CavanUdpCmdCallback
	Next     *CavanUdpCmdNode
	Link     *CavanUdpLink
	Time     time.Time
	Times    int
	Index    int
	DoneChan chan *CavanUdpRspNode
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
	node.DoneChan = make(chan *CavanUdpRspNode)
	return &node
}

func (command *CavanUdpCommand) WriteTo(link *CavanUdpLink, conn *net.UDPConn) (int, error) {
	return conn.WriteToUDP(command.Bytes, link.Addr)
}

func (command *CavanUdpCommand) Prepare(link *CavanUdpLink, times int) bool {
	return times < 5
}

func (command *CavanUdpCommand) Setup(link *CavanUdpLink, index int) {
	command.SetDestPort(int(link.RemotePort))
	command.SetSrcPort(int(link.LocalPort))
	command.SetIndex(uint8(index))
}

func NewCavanUdpCmdBuilder(length int) *CavanUdpCmdBuilder {
	bytes := make([]byte, length+6)
	builder := CavanUdpCmdBuilder{}
	builder.CavanUdpPack.Bytes = bytes
	builder.ByteArrayBuilder.Init(bytes, 6)
	return &builder
}

func (builder *CavanUdpCmdBuilder) Build(link *CavanUdpLink) *CavanUdpCmdNode {
	bytes := builder.ByteArrayBuilder.Build()
	command := CavanUdpCommand{}
	command.Bytes = bytes
	return NewCavanUdpCmdNode(link, &command)
}
