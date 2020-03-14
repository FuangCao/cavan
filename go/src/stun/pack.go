package stun

import (
	"fmt"
	"net"

	"../common"
)

type StunPack struct {
	ID    int
	Attrs []*StunAttr
}

func (pack *StunPack) Encode() []byte {
	length := 0

	for _, attr := range pack.Attrs {
		length += attr.Size()
	}

	bytes := make([]byte, length+20)
	common.EncodeValueBe16(bytes, 0, pack.ID)
	common.EncodeValueBe16(bytes, 2, length)

	index := 20

	for _, attr := range pack.Attrs {
		index = attr.Encode(bytes, index)
	}

	return bytes
}

func StunPackDecode(bytes []byte, length int) *StunPack {
	if length < 20 || length != common.DecodeValueBe16(bytes, 2)+20 {
		return nil
	}

	attrs := make([]*StunAttr, 0)
	index := 20

	for index < length {
		attr := DecodeStunAttr(bytes, index, length-index)
		if attr == nil {
			break
		}

		attrs = append(attrs, attr)
		index += attr.Size()
	}

	id := common.DecodeValueBe16(bytes, 0)

	return &StunPack{ID: id, Attrs: attrs}
}

func (pack *StunPack) String() string {
	for _, attr := range pack.Attrs {
		fmt.Println(attr.UDPAddr())
	}

	return fmt.Sprintf("ID = %04x", pack.ID)
}

func (pack *StunPack) GetWanAddr() *net.UDPAddr {
	for _, attr := range pack.Attrs {
		if attr.ID == 0x0001 {
			return attr.UDPAddr()
		}
	}

	return nil
}
