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
        private int mUsed;

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

            int total = mUsed + length;

            if (total > mBytes.Length)
            {
                byte[] newBytes = new byte[total * 2];
                Array.Copy(mBytes, newBytes, mUsed);
                mBytes = newBytes;
            }

            Array.Copy(bytes, offset, mBytes, mUsed, length);
            mUsed += length;

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

        public void write(Stream stream)
        {
            while (true)
            {
                int remain = mBytes.Length - mUsed;
                if (remain > 0)
                {
                    int rdlen = stream.Read(mBytes, mUsed, remain);
                    if (rdlen > 0)
                    {
                        mUsed += rdlen;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    byte[] bytes = new byte[mBytes.Length * 2];
                    Array.Copy(mBytes, bytes, mUsed);
                    mBytes = bytes;
                }
            }
        }

        public void clear()
        {
            mUsed = 0;
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
                return mUsed;
            }
        }

        public byte[] toBytes()
        {
            lock (this)
            {
                byte[] bytes = new byte[mUsed];
                Array.Copy(mBytes, bytes, mUsed);
                return bytes;
            }
        }

        public override string ToString()
        {
            return mEncoding.GetString(mBytes, 0, mUsed);
        }
    }
}
