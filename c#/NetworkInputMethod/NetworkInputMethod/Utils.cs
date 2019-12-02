using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace NetworkInputMethod
{
    public class Utils
    {
        public static int DecodeValue8(byte[] bytes, int index)
        {
            if (index < bytes.Length)
            {
                return bytes[index] & 0xFF;
            }

            return 0;
        }

        public static int DecodeValue16(byte[] bytes, int index)
        {
            return DecodeValue8(bytes, index + 1) << 8 | DecodeValue8(bytes, index);
        }

        public static int DecodeValue24(byte[] bytes, int index)
        {
            return DecodeValue16(bytes, index + 1) << 8 | DecodeValue8(bytes, index);
        }

        public static int DecodeValue32(byte[] bytes, int index)
        {
            return DecodeValue24(bytes, index + 1) << 8 | DecodeValue8(bytes, index);
        }

        public static void EncodeValue8(byte[] bytes, int index, int value)
        {
            bytes[index] = (byte)value;
        }

        public static void EncodeValue16(byte[] bytes, int index, int value)
        {
            EncodeValue8(bytes, index, value);
            EncodeValue8(bytes, index + 1, value >> 8);
        }

        public static void EncodeValue24(byte[] bytes, int index, int value)
        {
            EncodeValue16(bytes, index, value);
            EncodeValue8(bytes, index + 2, value >> 16);
        }

        public static void EncodeValue32(byte[] bytes, int index, int value)
        {
            EncodeValue24(bytes, index, value);
            EncodeValue8(bytes, index + 3, value >> 24);
        }
    }
}
