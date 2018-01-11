using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
using System.IO;
using System.Threading;

namespace NetworkInputMethod
{
    public class CavanHttpPacket
    {
        private static char[] sHeaderSplitChars = new char[] { ':' };

        private ByteArrayWriter mWriter = new ByteArrayWriter();
        private Hashtable mHeaders = new Hashtable();
        private byte[] mBody;
        int mLines;

        public ByteArrayWriter Writer
        {
            get
            {
                return mWriter;
            }
        }

        public void writeLine()
        {
            mWriter.write("\r\n");
            mLines++;
        }

        public void writeLine(string line)
        {
            mWriter.write(line);
            writeLine();
        }

        public bool addLine(string line)
        {
            if (line == null || line.Length == 0)
            {
                if (mLines > 0)
                {
                    writeLine();
                    return true;
                }

                return false;
            }

            writeLine(line);

            if (mLines > 1)
            {
                string[] args = line.Split(sHeaderSplitChars, 2);
                if (args.Length == 2)
                {
                    mHeaders[args[0].Trim().ToLower()] = args[1].TrimStart();
                }
            }

            return false;
        }

        public string getHeader(string name)
        {
            return mHeaders[name] as string;
        }

        public void addHeader(string name, string value)
        {
            writeLine(name + ": " + value);
            mHeaders[name.ToLower()] = value;
        }

        public Hashtable Headers
        {
            get
            {
                return mHeaders;
            }
        }

        public int Lines
        {
            get
            {
                return mLines;
            }
        }

        public int HeaderCount
        {
            get
            {
                return mHeaders.Count;
            }
        }

        public byte[] Body
        {
            get
            {
                return mBody;
            }

            set
            {
                mBody = value;
            }
        }

        public string getHost()
        {
            return getHeader("host");
        }

        public int getContentLength()
        {
            string text = getHeader("content-length");
            if (text == null)
            {
                return -1;
            }

            return Convert.ToInt32(text);
        }

        public string getConnection()
        {
            return getHeader("connection");
        }

        public string getContentEncoding()
        {
            return getHeader("content-encoding");
        }

        public string getContentType()
        {
            return getHeader("content-type");
        }

        public bool writeTo(Stream stream)
        {
            try
            {
                mWriter.writeTo(stream);
                stream.Flush();
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

        public bool readFrom(Stream stream)
        {
            mBody = null;
            mWriter.clear();
            mHeaders.Clear();

            CavanHttpReader reader = new CavanHttpReader(stream);

            while (true)
            {
                string line = reader.ReadLine();
                if (line == null)
                {
                    return false;
                }

                if (addLine(line))
                {
                    break;
                }
            }

            int length = getContentLength();
            if (length > 0)
            {
                mBody = reader.ReadBytes(getContentEncoding(), length);
            }
            else
            {
                mBody = null;
            }

            return true;
        }

        public bool isNeedClose()
        {
            string conntction = getConnection();
            return (conntction != null && conntction.Equals("close", StringComparison.CurrentCultureIgnoreCase));
        }
    }
}
