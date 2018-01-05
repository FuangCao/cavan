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

        private List<string> mLines = new List<string>();
        private Hashtable mHeaders = new Hashtable();
        private byte[] mBody;

        public bool addLine(string line)
        {
            if (line == null || line.Length == 0)
            {
                return true;
            }

            mLines.Add(line);

            string[] args = line.Split(sHeaderSplitChars, 2);
            if (args.Length == 2)
            {
                mHeaders[args[0].Trim().ToLower()] = args[1].TrimStart();
            }

            return false;
        }

        public string getHeader(string name)
        {
            return mHeaders[name] as string;
        }

        public void addHeader(string name, string value)
        {
            mLines.Add(name + ": " + value);
            mHeaders[name.ToLower()] = value;
        }

        public List<string> Lines
        {
            get
            {
                return mLines;
            }

            set
            {
                mLines.Clear();
                mHeaders.Clear();

                foreach (string line in value)
                {
                    addLine(line);
                }
            }
        }

        public Hashtable Headers
        {
            get
            {
                return mHeaders;
            }
        }

        public int LineCount
        {
            get
            {
                return mLines.Count;
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
            CavanHttpWriter writer = new CavanHttpWriter(stream);

            foreach (string line in mLines)
            {
                if (!writer.WriteLine(line))
                {
                    return false;
                }
            }

            if (!writer.WriteLine())
            {
                return false;
            }

            if (mBody != null && mBody.Length > 0)
            {
                if (!writer.WriteBytes(mBody, 0, mBody.Length))
                {
                    return false;
                }
            }

            return writer.Flush();
        }
        public bool writeTo(Stream stream, ByteArrayWriter writer)
        {
            if (writer == null)
            {
                return writeTo(stream);
            }

            try
            {
                stream.Write(writer.getBytes(), 0, writer.getLength());
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
            mLines.Clear();
            mHeaders.Clear();

            CavanHttpReader reader = new CavanHttpReader(stream);

            while (true)
            {
                string line = reader.ReadLine();
                if (line == null)
                {
                    break;
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

        public ByteArrayWriter buildBytes()
        {
            ByteArrayWriter writer = new ByteArrayWriter();

            foreach (string line in mLines)
            {
                writer.write(line);
                writer.write("\r\n");
            }

            writer.write("\r\n");

            if (mBody != null && mBody.Length > 0)
            {
                writer.write(mBody);
            }

            return writer;
        }

        public bool isNeedClose()
        {
            string conntction = getConnection();
            return (conntction != null && conntction.Equals("close", StringComparison.CurrentCultureIgnoreCase));
        }
    }
}
