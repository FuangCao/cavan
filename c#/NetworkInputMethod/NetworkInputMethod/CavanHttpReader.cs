using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.IO.Compression;

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
            byte[] bytes = new byte[4096];
            int offset = 0;

            while (true)
            {
                int value = mStream.ReadByte();
                if (value < 0)
                {
                    if (offset > 0)
                    {
                        break;
                    }

                    return null;
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

        public byte[] ReadBytes(int length)
        {
            byte[] bytes = new byte[length];
            if (ReadBytes(bytes, 0, length))
            {
                return bytes;
            }

            return null;
        }

        public byte[] ReadBytes(string encoding, int length)
        {
            byte[] bytes = ReadBytes(length);
            if (bytes == null)
            {
                return null;
            }

            if (encoding != null)
            {
                Console.WriteLine("encoding = " + encoding);

                using (MemoryStream mstream = new MemoryStream(bytes))
                {
                    Stream dstream;

                    if (encoding.Equals("gzip", StringComparison.CurrentCultureIgnoreCase))
                    {
                        dstream = new GZipStream(mstream, CompressionMode.Decompress, true);
                    }
                    else if (encoding.Equals("deflate", StringComparison.CurrentCultureIgnoreCase))
                    {
                        dstream = new DeflateStream(mstream, CompressionMode.Decompress, true);
                    }
                    else
                    {
                        dstream = null;
                    }

                    if (dstream != null)
                    {
                        ByteArrayWriter writer = new ByteArrayWriter();
                        writer.readFrom(dstream);
                        dstream.Close();
                        bytes = writer.toBytes();
                    }
                }
            }

            return bytes;
        }
    }
}
