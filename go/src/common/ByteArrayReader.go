package common

import (
	"net"
)

type ByteArrayReader struct {
	Bytes    []byte
	Position int
}

func (reader *ByteArrayReader) Seek(position int) {
	reader.Position = position
}

func (reader *ByteArrayReader) Skip(offset int) {
	reader.Position += offset
}

func (reader *ByteArrayReader) Remain() int {
	return len(reader.Bytes) - reader.Position
}

func (reader *ByteArrayReader) Byte() byte {
	index := reader.Position

	if index < len(reader.Bytes) {
		reader.Position = index + 1
		return reader.Bytes[index]
	}

	return 0
}

func (reader *ByteArrayReader) Value8() int {
	return int(reader.Byte())
}

func (reader *ByteArrayReader) Value16() int {
	return reader.Value8() | reader.Value8()<<8
}

func (reader *ByteArrayReader) Value24() int {
	return reader.Value16() | reader.Value8()<<16
}

func (reader *ByteArrayReader) Value32() int {
	return reader.Value24() | reader.Value8()<<24
}

func (reader *ByteArrayReader) ValueBe16() int {
	return reader.Value8()<<8 | reader.Value8()
}

func (reader *ByteArrayReader) ValueBe24() int {
	return reader.ValueBe16()<<8 | reader.Value8()
}

func (reader *ByteArrayReader) ValueBe32() int {
	return reader.ValueBe24()<<8 | reader.Value8()
}

func (reader *ByteArrayReader) Port() int {
	return reader.ValueBe16()
}

func (reader *ByteArrayReader) IP() net.IP {
	return net.IPv4(reader.Byte(), reader.Byte(), reader.Byte(), reader.Byte())
}

func (reader *ByteArrayReader) UDPAddr() *net.UDPAddr {
	port := reader.Port()
	ip := reader.IP()
	return &net.UDPAddr{IP: ip, Port: port}
}
