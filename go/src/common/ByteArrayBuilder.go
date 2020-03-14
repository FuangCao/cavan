package common

type ByteArrayBuilder struct {
	Bytes    []byte
	Position int
}

func NewByteArrayBuilder(length int) *ByteArrayBuilder {
	builder := ByteArrayBuilder{}
	builder.Bytes = make([]byte, length)
	return &builder
}

func (builder *ByteArrayBuilder) Init(bytes []byte, position int) {
	builder.Bytes = bytes
	builder.Position = position
}

func (builder *ByteArrayBuilder) Build() []byte {
	if len(builder.Bytes) == builder.Position {
		return builder.Bytes
	}

	bytes := make([]byte, builder.Position)
	copy(bytes, builder.Bytes[:builder.Position])

	return bytes
}

func (builder *ByteArrayBuilder) Seek(offset int) {
	builder.Position += offset
}

func (builder *ByteArrayBuilder) AppendValue8(value uint8) {
	builder.Bytes[builder.Position] = byte(value)
	builder.Position++
}

func (builder *ByteArrayBuilder) AppendValue16(value uint16) {
	builder.AppendValue8(uint8(value))
	builder.AppendValue8(uint8(value >> 8))
}

func (builder *ByteArrayBuilder) AppendValue24(value uint32) {
	builder.AppendValue16(uint16(value))
	builder.AppendValue8(uint8(value >> 16))
}

func (builder *ByteArrayBuilder) AppendValue32(value uint32) {
	builder.AppendValue24(value)
	builder.AppendValue8(uint8(value >> 24))
}

func (builder *ByteArrayBuilder) AppendBytes(bytes []byte) {
	copy(builder.Bytes[builder.Position:], bytes)
	builder.Position += len(bytes)
}
