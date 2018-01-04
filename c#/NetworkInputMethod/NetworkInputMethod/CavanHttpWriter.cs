using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Threading;

namespace NetworkInputMethod
{
    public class CavanHttpWriter
    {
        private Stream mStream;

        public CavanHttpWriter(Stream stream)
        {
            mStream = stream;
        }

        public bool WriteBytes(byte[] bytes, int offset, int length)
        {
            try
            {
                mStream.Write(bytes, offset, length);
            }
            catch (ThreadAbortException)
            {
                throw;
            }
            catch (Exception)
            {
                return false;
            }

            return true;
        }

        public bool WriteText(string text)
        {
            byte[] bytes = Encoding.UTF8.GetBytes(text);
            return WriteBytes(bytes, 0, bytes.Length);
        }

        public bool WriteLine()
        {
            return WriteText("\r\n");
        }

        public bool WriteLine(string line)
        {
            return WriteText(line) && WriteLine();
        }

        public bool Flush()
        {
            try
            {
                mStream.Flush();
            }
            catch (ThreadAbortException)
            {
                throw;
            }
            catch (Exception)
            {
                return false;
            }

            return true;
        }
    }
}
