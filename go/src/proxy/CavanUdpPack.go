package proxy

import (
	"../common"
)

type CavanUdpPackType uint8

const (
	CavanUdpCmdPing    CavanUdpPackType = 0
	CavanUdpCmdConn    CavanUdpPackType = 1
	CavanUdpCmdLink    CavanUdpPackType = 2
	CavanUdpCmdData    CavanUdpPackType = 3
	CavanUdpCmdClose   CavanUdpPackType = 4
	CavanUdpRspSuccess CavanUdpPackType = 8
	CavanUdpRspError   CavanUdpPackType = 9
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

func (pack *CavanUdpPack) DestPort() int {
	return common.DecodeValue16(pack.Bytes, 0)
}

func (pack *CavanUdpPack) SetDestPort(value int) {
	common.EncodeValue16(pack.Bytes, 0, value)
}

func (pack *CavanUdpPack) SrcPort() int {
	return common.DecodeValue16(pack.Bytes, 2)
}

func (pack *CavanUdpPack) SetSrcPort(value int) {
	common.EncodeValue16(pack.Bytes, 2, value)
}

func (pack *CavanUdpPack) Index() uint8 {
	return pack.Bytes[4]
}

func (pack *CavanUdpPack) SetIndex(value uint8) {
	pack.Bytes[4] = value
}

func (pack *CavanUdpPack) Type() CavanUdpPackType {
	return CavanUdpPackType(pack.Bytes[5])
}

func (pack *CavanUdpPack) SetType(value CavanUdpPackType) {
	pack.Bytes[5] = byte(value)
}

func (pack *CavanUdpPack) Body() []byte {
	return pack.Bytes[6:]
}

func (pack *CavanUdpPack) SetupWithLink(link *CavanUdpLink) {
	pack.SetDestPort(int(link.RemotePort))
	pack.SetSrcPort(int(link.LocalPort))
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
