using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace NetworkInputMethod
{
    public class CavanHttpReader
    {
        private Stream mStream;

        public CavanHttpReader(Stream stream)
        {
            mStream = stream;
        }

        public string ReadLine()
        {
            byte[] bytes = new byte[1024];
            int offset = 0;

            while (true)
            {
                int value = mStream.ReadByte();
                if (value < 0)
                {
                    break;
                }

                if (value == '\n')
                {
                    break;
                }
                else if (value != '\r')
                {
                    if (offset >= bytes.Length)
                    {
                        byte[] newBytes = new byte[bytes.Length << 1];
                        Array.Copy(bytes, newBytes, bytes.Length);
                        bytes = newBytes;
                    }

                    bytes[offset++] = (byte)value;
                }
            }

            return Encoding.UTF8.GetString(bytes, 0, offset);
        }

        public bool ReadBytes(byte[] bytes, int offset, int length)
        {
            while (length > 0)
            {
                int rdlen = mStream.Read(bytes, offset, length);
                if (rdlen < 0)
                {
                    return false;
                }

                offset += rdlen;
                length -= rdlen;
            }

            return true;
        }
    }
}
