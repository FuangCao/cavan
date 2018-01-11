using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.IO;

namespace NetworkInputMethod
{
    public class ByteArrayWriter
    {
        private Encoding mEncoding = Encoding.UTF8;
        private byte[] mBytes = new byte[4096];
        private int mLength;

        public void setEncoding(Encoding encoding)
        {
            lock (this)
            {
                mEncoding = encoding;
            }
        }

        public void write(byte[] bytes, int offset, int length)
        {
            Monitor.Enter(this);

            int total = mLength + length;

            if (total > mBytes.Length)
            {
                byte[] newBytes = new byte[total * 2];
                Array.Copy(mBytes, newBytes, mLength);
                mBytes = newBytes;
            }

            Array.Copy(bytes, offset, mBytes, mLength, length);
            mLength = total;

            Monitor.Exit(this);
        }

        public void write(byte[] bytes, int offset)
        {
            write(bytes, 0, bytes.Length);
        }

        public void write(byte[] bytes)
        {
            write(bytes, 0, bytes.Length);
        }

        public void write(string text)
        {
            byte[] bytes = mEncoding.GetBytes(text);
            write(bytes, 0, bytes.Length);
        }

        public void readFrom(Stream stream)
        {
            while (true)
            {
                int remain = mBytes.Length - mLength;
                if (remain > 0)
                {
                    int rdlen = stream.Read(mBytes, mLength, remain);
                    if (rdlen > 0)
                    {
                        mLength += rdlen;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    byte[] bytes = new byte[mBytes.Length * 2];
                    Array.Copy(mBytes, bytes, mLength);
                    mBytes = bytes;
                }
            }
        }

        public void writeTo(Stream stream)
        {
            stream.Write(mBytes, 0, mLength);
        }

        public void clear()
        {
            mLength = 0;
        }

        public byte[] getBytes()
        {
            lock (this)
            {
                return mBytes;
            }
        }

        public int getLength()
        {
            lock (this)
            {
                return mLength;
            }
        }

        public byte[] toBytes()
        {
            lock (this)
            {
                byte[] bytes = new byte[mLength];
                Array.Copy(mBytes, bytes, mLength);
                return bytes;
            }
        }

        public override string ToString()
        {
            return mEncoding.GetString(mBytes, 0, mLength);
        }
    }
}
