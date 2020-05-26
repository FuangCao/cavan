package proxy

import (
	"net"
	"fmt"

	"../common"
)

var StunServerUrls = []string {
	"stun.l.google.com:19302",
	"stun1.l.google.com:19302",
	"stun.xten.com:3478",
	"stunserver.org:3478",
	"stun.ekiga.net:3478",
	"stun.voipstunt.com:3478",
	"stun.voiparound.com:3478",
	"stun.voip.aebc.com:3478",
	"stun.sipgate.net:3478",
	"stun.internetcalls.com:3478",
}

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
	addr := command.Addr
	if addr == nil {
		return 0, nil
	}

	return conn.WriteToUDP(command.Bytes, addr)
}

func (command *CavanStunCommand) Prepare(link *CavanUdpLink, times int) bool {
	if times > len(StunServerUrls) * 3 {
		return false
	}

	url := StunServerUrls[times % len(StunServerUrls)]
	fmt.Println("stun url =", url)

	if addr, err := net.ResolveUDPAddr("udp", url); err == nil {
		fmt.Println("stun addr =", addr)
		command.Addr = addr
	}

	return true
}

func (command *CavanStunCommand) Setup(index uint8) {
}

func (command *CavanStunCommand) RspOpCode() CavanUdpOpCode {
	return 0
}

func (command *CavanStunCommand) MatchAck(pack *CavanUdpPack) bool {
	return true
}
