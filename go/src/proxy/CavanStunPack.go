package proxy

import (
	"net"

	"../common"
)

type CavanStunPack struct {
	Bytes []byte
}

func (pack *CavanStunPack) Type() int {
	return common.DecodeValueBe16(pack.Bytes, 0)
}

func (pack *CavanStunPack) SetType(value int) {
	common.EncodeValueBe16(pack.Bytes, 0, value)
}

func (pack *CavanStunPack) Length() int {
	return common.DecodeValueBe16(pack.Bytes, 2)
}

func (pack *CavanStunPack) SetLength(length int) {
	common.EncodeValueBe16(pack.Bytes, 2, length)
}

func (pack *CavanStunPack) GetWanAddr() *net.UDPAddr {
	reader := common.ByteArrayReader{Bytes: pack.Bytes}
	reader.Seek(20)

	for reader.Position < len(pack.Bytes) {
		id := reader.ValueBe16()
		length := reader.ValueBe16()

		if id == 1 && length == 8 {
			reader.Skip(2)
			return reader.UDPAddr()
		}

		if length == 0 {
			break
		}

		reader.Skip(length)
	}

	return nil
}
