package common

import (
	"net"
	"time"
)

func EncodeValue8(bytes []byte, index int, value int) {
	bytes[index] = byte(value)
}

func EncodeValue16(bytes []byte, index int, value int) {
	EncodeValue8(bytes, index, value)
	EncodeValue8(bytes, index+1, value>>8)
}

func EncodeValue24(bytes []byte, index int, value int) {
	EncodeValue16(bytes, index, value)
	EncodeValue8(bytes, index+2, value>>16)
}

func EncodeValue32(bytes []byte, index int, value int) {
	EncodeValue24(bytes, index, value)
	EncodeValue8(bytes, index+3, value>>24)
}

func DecodeValue8(bytes []byte, index int) int {
	return int(bytes[index])
}

func DecodeValue16(bytes []byte, index int) int {
	return DecodeValue8(bytes, index) | DecodeValue8(bytes, index+1)<<8
}

func DecodeValue24(bytes []byte, index int) int {
	return DecodeValue16(bytes, index) | DecodeValue8(bytes, index+2)<<16
}

func DecodeValue32(bytes []byte, index int) int {
	return DecodeValue24(bytes, index) | DecodeValue8(bytes, index+3)<<24
}

func EncodeValueBe16(bytes []byte, index int, value int) {
	EncodeValue8(bytes, index, value>>8)
	EncodeValue8(bytes, index+1, value)
}

func EncodeValueBe24(bytes []byte, index int, value int) {
	EncodeValue8(bytes, index, value>>16)
	EncodeValueBe16(bytes, index+1, value)
}

func EncodeValueBe32(bytes []byte, index int, value int) {
	EncodeValue8(bytes, index, value>>24)
	EncodeValueBe24(bytes, index+1, value)
}

func DecodeValueBe16(bytes []byte, index int) int {
	return DecodeValue8(bytes, index)<<8 | DecodeValue8(bytes, index+1)
}

func DecodeValueBe24(bytes []byte, index int) int {
	return DecodeValueBe16(bytes, index)<<8 | DecodeValue8(bytes, index+2)
}

func DecodeValueBe32(bytes []byte, index int) int {
	return DecodeValueBe24(bytes, index)<<8 | DecodeValue8(bytes, index+3)
}

func Clone(bytes []byte, length int) []byte {
	clone := make([]byte, length)
	copy(clone, bytes[0:length])
	return clone
}

func CavanConnFill(conn net.Conn, bytes []byte) error {
	for true {
		length, err := conn.Read(bytes)
		if err != nil {
			return err
		}

		size := len(bytes)

		if length < size {
			bytes = bytes[length : size-length]
		} else {
			break
		}
	}

	return nil
}

func CavanConnReadPack(conn net.Conn) ([]byte, error) {
	conn.SetReadDeadline(time.Now().Add(time.Minute))

	header := make([]byte, 2)

	if err := CavanConnFill(conn, header); err != nil {
		return nil, err
	}

	size := DecodeValue16(header, 0)
	bytes := make([]byte, size)

	if size > 0 {
		if err := CavanConnFill(conn, bytes); err != nil {
			return nil, err
		}
	}

	return bytes, nil
}

func CavanConnWriteAll(conn net.Conn, bytes []byte) error {
	for true {
		length, err := conn.Write(bytes)
		if err != nil {
			return err
		}

		size := len(bytes)

		if length < size {
			bytes = bytes[length : size-length]
		} else {
			break
		}
	}

	return nil
}

func CavanConnWritePack(conn net.Conn, bytes []byte) error {
	header := make([]byte, 2)
	EncodeValue16(header, 0, len(bytes))

	if err := CavanConnWriteAll(conn, header); err != nil {
		return err
	}

	if err := CavanConnWriteAll(conn, bytes); err != nil {
		return err
	}

	return nil
}
