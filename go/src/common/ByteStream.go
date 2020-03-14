package common

type ByteStream struct {
	Bytes []byte
	Index int
}

func NewByteStream(length int) *ByteStream {
	stream := &ByteStream{Bytes: make([]byte, length)}
	return stream
}

func (stream *ByteStream) ReadValue8() uint8 {
	index := stream.Index
	stream.Index = index + 1
	return stream.Bytes[index]
}

func (stream *ByteStream) ReadValue16() uint16 {
	return uint16(stream.ReadValue8()) | uint16(stream.ReadValue8())<<8
}

func (stream *ByteStream) ReadValue24() uint32 {
	return uint32(stream.ReadValue16()) | uint32(stream.ReadValue8())<<16
}

func (stream *ByteStream) ReadValue32() uint32 {
	return stream.ReadValue24() | uint32(stream.ReadValue8())<<24
}

func (stream *ByteStream) ReadValueBe16() uint16 {
	return uint16(stream.ReadValue8())<<8 | uint16(stream.ReadValue8())
}

func (stream *ByteStream) ReadValueBe24() uint32 {
	return uint32(stream.ReadValue16())<<16 | uint32(stream.ReadValue8())
}

func (stream *ByteStream) ReadValueBe32() uint32 {
	return stream.ReadValueBe24()<<24 | uint32(stream.ReadValue8())
}

func (stream *ByteStream) ReadBytes(length int) []byte {
	index := stream.Index
	stream.Index = index + length
	return stream.Bytes[index:stream.Index]
}

func (stream *ByteStream) ReadBytesToEnd() []byte {
	return stream.ReadBytes(len(stream.Bytes) - stream.Index)
}

func (stream *ByteStream) WriteValue8(value uint8) {
	index := stream.Index
	stream.Index = index + 1
	stream.Bytes[index] = value
}

func (stream *ByteStream) WriteValue16(value uint16) {
	stream.WriteValue8(uint8(value))
	stream.WriteValue8(uint8(value >> 8))
}

func (stream *ByteStream) WriteValueBe16(value uint16) {
	stream.WriteValue8(uint8(value >> 8))
	stream.WriteValue8(uint8(value))
}

func (stream *ByteStream) WriteValue24(value uint32) {
	stream.WriteValue16(uint16(value))
	stream.WriteValue8(uint8(value >> 16))
}

func (stream *ByteStream) WriteValueBe24(value uint32) {
	stream.WriteValue8(uint8(value >> 16))
	stream.WriteValueBe16(uint16(value))
}

func (stream *ByteStream) WriteValue32(value uint32) {
	stream.WriteValue24(value)
	stream.WriteValue8(uint8(value >> 24))
}

func (stream *ByteStream) WriteValueBe32(value uint32) {
	stream.WriteValue8(uint8(value >> 24))
	stream.WriteValueBe24(value)
}

func (stream *ByteStream) WriteBytes(bytes []byte) {
	index := stream.Index
	stream.Index = index + len(bytes)
	copy(stream.Bytes[index:stream.Index], bytes)
}

func (stream *ByteStream) Seek(position int) {
	stream.Index = position
}

func (stream *ByteStream) Skip(offset int) {
	stream.Index += offset
}
