package proxy

import (
	"../common"
)

type CavanUdpOpCode uint8

const (
	CavanUdpOpAck   CavanUdpOpCode = 0
	CavanUdpOpPing  CavanUdpOpCode = 1
	CavanUdpOpConn  CavanUdpOpCode = 2
	CavanUdpOpLink  CavanUdpOpCode = 3
	CavanUdpOpData  CavanUdpOpCode = 4
	CavanUdpOpClose CavanUdpOpCode = 5
)

type CavanUdpPack struct {
	Bytes []byte
}

type CavanUdpPackBuilder struct {
	common.ByteArrayBuilder
	CavanUdpPack
}

func NewCavanUdpPack(bytes []byte) *CavanUdpPack {
	pack := CavanUdpPack{Bytes: bytes}
	return &pack
}

func (pack *CavanUdpPack) DestPort() uint16 {
	return uint16(common.DecodeValue16(pack.Bytes, 0))
}

func (pack *CavanUdpPack) SetDestPort(value uint16) {
	common.EncodeValue16(pack.Bytes, 0, int(value))
}

func (pack *CavanUdpPack) SrcPort() uint16 {
	return uint16(common.DecodeValue16(pack.Bytes, 2))
}

func (pack *CavanUdpPack) SetSrcPort(value uint16) {
	common.EncodeValue16(pack.Bytes, 2, int(value))
}

func (pack *CavanUdpPack) Index() uint8 {
	return pack.Bytes[4]
}

func (pack *CavanUdpPack) SetIndex(value uint8) {
	pack.Bytes[4] = value
}

func (pack *CavanUdpPack) OpCode() CavanUdpOpCode {
	return CavanUdpOpCode(pack.Bytes[5])
}

func (pack *CavanUdpPack) SetOpCode(op CavanUdpOpCode) {
	pack.Bytes[5] = byte(op)
}

func (pack *CavanUdpPack) Body() []byte {
	return pack.Bytes[6:]
}

func (pack *CavanUdpPack) SetupWithLink(link *CavanUdpLink) {
	pack.SetDestPort(link.RemotePort)
	pack.SetSrcPort(link.LocalPort)
}

func (pack *CavanUdpPack) SetupWithCommand(command *CavanUdpPack) {
	pack.SetDestPort(command.SrcPort())
	pack.SetSrcPort(command.DestPort())
	pack.SetIndex(command.Index())
}

func (builder *CavanUdpPackBuilder) Init(length int) *CavanUdpPackBuilder {
	bytes := make([]byte, length+6)
	builder.ByteArrayBuilder.Init(bytes, 6)
	builder.CavanUdpPack.Bytes = bytes
	return builder
}

func (builder *CavanUdpPackBuilder) Build() *CavanUdpPack {
	pack := CavanUdpPack{Bytes: builder.ByteArrayBuilder.Build()}
	return &pack
}
