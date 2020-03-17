package proxy

import (
	"net"

	"../common"
)

type CavanStunCommand struct {
	CavanStunPack
	Addr *net.UDPAddr
}

type CavanStunCmdBuilder struct {
	common.ByteArrayBuilder
	CavanStunPack
}

func NewCavanStunCmdBuilder(length int) *CavanStunCmdBuilder {
	bytes := make([]byte, length+20)
	builder := CavanStunCmdBuilder{}
	builder.CavanStunPack.Bytes = bytes
	builder.ByteArrayBuilder.Init(bytes, 20)
	builder.SetLength(length)
	return &builder
}

func (builder *CavanStunCmdBuilder) Build(link *CavanUdpLink) *CavanUdpCmdNode {
	bytes := builder.ByteArrayBuilder.Build()
	command := CavanStunCommand{}
	command.Bytes = bytes
	return NewCavanUdpCmdNode(link, &command)
}

func (command *CavanStunCommand) WriteTo(link *CavanUdpLink, conn *net.UDPConn) (int, error) {
	if command.Addr == nil {
		return 0, nil
	}

	return conn.WriteToUDP(command.Bytes, command.Addr)
}

func (command *CavanStunCommand) Prepare(link *CavanUdpLink, times int) bool {
	if times > 10 {
		return false
	}

	if addr, err := net.ResolveUDPAddr("udp", "stun1.l.google.com:19302"); err == nil {
		command.Addr = addr
	}

	return true
}

func (command *CavanStunCommand) Setup(index uint8) {
}

func (command *CavanStunCommand) RspOpCode() CavanUdpOpCode {
	return 0
}
