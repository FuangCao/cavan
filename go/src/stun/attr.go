package stun

import (
	"net"

	"../common"
)

type StunAttr struct {
	ID    int
	Value []byte
}

func NewStunAttr(id int, value []byte) *StunAttr {
	return &StunAttr{ID: id, Value: value}
}

func DecodeStunAttr(bytes []byte, index int, size int) *StunAttr {
	if size < 4 {
		return nil
	}

	length := common.DecodeValueBe16(bytes, index+2)
	if size < length+4 {
		return nil
	}

	return NewStunAttr(common.DecodeValueBe16(bytes, index), bytes[index+4:index+4+length])
}

func (attr *StunAttr) Encode(bytes []byte, index int) int {
	common.EncodeValueBe16(bytes, index, len(attr.Value))
	common.EncodeValueBe16(bytes, index+2, attr.ID)
	copy(bytes[index+4:], attr.Value)
	return index + len(attr.Value) + 4
}

func (attr *StunAttr) Size() int {
	return len(attr.Value) + 4
}

func (attr *StunAttr) Port() int {
	return common.DecodeValueBe16(attr.Value, 2)
}

func (attr *StunAttr) Host() net.IP {
	return net.IPv4(attr.Value[4], attr.Value[5], attr.Value[6], attr.Value[7])
}

func (attr StunAttr) UDPAddr() *net.UDPAddr {
	return &net.UDPAddr{IP: attr.Host(), Port: attr.Port()}
}
