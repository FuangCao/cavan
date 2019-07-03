using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Windows.Forms;
using NetworkInputMethod.Properties;
using System.Collections;

namespace NetworkInputMethod
{
    public partial class FormHttpCapture : FormTcpService
    {
        private CavanTcpService mService;

        delegate void PacketCapturedHandler(HttpCapturePacket req, HttpCapturePacket rsp);

        public FormHttpCapture()
        {
            mService = new CavanTcpService(this);
            InitializeComponent();
            textBoxPort.Text = Settings.Default.HttpCapturePort.ToString();
        }

        private void buttonSwitch_Click(object sender, EventArgs e)
        {
            mService.toggle(textBoxPort.Text);
        }

        public override void onTcpServiceStarted(object sender, EventArgs e)
        {
            buttonSwitch.Text = "停止";
        }

        public override void onTcpServiceRunning(object sender, EventArgs e)
        {
            Settings.Default.HttpCapturePort = mService.Port;
        }

        public override void onTcpServiceStopped(object sender, EventArgs e)
        {
            buttonSwitch.Text = "启动";
        }

        public override CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            return new HttpCaptureClient(conn);
        }

        private void FormHttpCapture_Load(object sender, EventArgs e)
        {
            if (Settings.Default.HttpCaptureEnable)
            {
                buttonSwitch.PerformClick();
            }
        }

        private void onPacketCaptured(HttpCapturePacket req, HttpCapturePacket rsp)
        {
            var packet = new HttpCapturePacketFull
            {
                Request = req,
                Response = rsp,
            };

            listBoxUrl.Items.Add(packet);
        }

        public void performPacketCaptured(HttpCapturePacket req, HttpCapturePacket rsp)
        {
            var method = new PacketCapturedHandler(onPacketCaptured);
            Invoke(method, req, rsp);
        }

        private void listBoxUrl_SelectedIndexChanged(object sender, EventArgs e)
        {
            var packet = listBoxUrl.SelectedItem as HttpCapturePacketFull;
            if (packet != null)
            {
                textBoxReq.Text = packet.Request.ToString();
                textBoxRsp.Text = packet.Response.ToString();
            }
        }
    }

    public enum HttpHeaderType
    {
        Host,
        ContentType,
        ContentLength,
    }

    public class HttpCapturePacket
    {
        public const int OVERTIME = 20000000;
        public const int CONTENT_MAX = 10 << 20;

        public static byte[] LineEnd = Encoding.UTF8.GetBytes("\r\n");
        public static Hashtable HeaderTable = new Hashtable()
        {
            { "host", HttpHeaderType.Host },
            { "content-type", HttpHeaderType.ContentType },
            { "content-length", HttpHeaderType.ContentLength },

        };

        public static string[] HeaderNames = new string[]
        {
            "Host",
            "Content-Type",
            "Content-Length",
        };

        private string mFirstLine;
        private ArrayList mLines;

        private byte[] mContent = new byte[1024];
        private int mLength;

        private string mMethod;
        private CavanUrl mUrl;
        private string mVersion;

        private string[] mHeaders = new string[HeaderNames.Length];

        public string FirstLine
        {
            get
            {
                return mFirstLine;
            }

            set
            {
                mFirstLine = value;
            }
        }

        public ArrayList Lines
        {
            get
            {
                return mLines;
            }

            set
            {
                mLines = value;
            }
        }

        public byte[] Content
        {
            get
            {
                return mContent;
            }

            set
            {
                mContent = value;
            }
        }

        public int Length
        {
            get
            {
                return mLength;
            }

            set
            {
                mLength = value;
            }
        }

        public string Method
        {
            get
            {
                return mMethod;
            }

            set
            {
                mMethod = value;
            }
        }

        public CavanUrl Url
        {
            get
            {
                return mUrl;
            }

            set
            {
                mUrl = value;
            }
        }

        public string Version
        {
            get
            {
                return mVersion;
            }

            set
            {
                mVersion = value;
            }
        }

        public int ContentLength
        {
            get
            {
                return getHeader(HttpHeaderType.ContentLength, -1);
            }

            set
            {
                setHeader(HttpHeaderType.ContentLength, value);
            }
        }

        public string getHeader(HttpHeaderType type)
        {
            return mHeaders[(int)type];
        }

        public int getHeader(HttpHeaderType type, int vdef)
        {
            var text = getHeader(type);
            if (text == null)
            {
                return vdef;
            }

            return Convert.ToInt32(text);
        }

        public string setHeader(HttpHeaderType type, object value)
        {
            var prev = mHeaders[(int)type];
            mHeaders[(int)type] = value.ToString();
            return prev;
        }

        public string readline(Socket sock, byte[] bytes)
        {
            var buff = new byte[1];
            var length = 0;

            while (true)
            {
                if (sock.Receive(buff) < 1)
                {
                    return null;
                }

                var value = buff[0];

                if (value == '\r')
                {
                    continue;
                }

                if (value == '\n')
                {
                    break;
                }

                if (length < bytes.Length)
                {
                    bytes[length++] = value;
                }
                else
                {
                    return null;
                }
            }

            return Encoding.UTF8.GetString(bytes, 0, length);
        }

        public bool read(Socket sock)
        {
            var bytes = new byte[4096];
            string line;

            for (int i = 0; i < mHeaders.Length; i++)
            {
                mHeaders[i] = null;
            }

            line = readline(sock, bytes);
            if (line == null)
            {
                return false;
            }

            mFirstLine = line;

            var lines = new ArrayList();

            while (true)
            {
                line = readline(sock, bytes);
                if (line == null)
                {
                    return false;
                }

                if (line.Length == 0)
                {
                    break;
                }

                lines.Add(line);

                var index = line.IndexOf(':');
                if (index < 0)
                {
                    continue;
                }

                var key = line.Substring(0, index).Trim();
                var header = HeaderTable[key.ToLower()];

                if (header != null)
                {
                    mHeaders[(int)header] = line.Substring(index + 1).TrimStart();
                }
            }

            for (int i = 0; i < mHeaders.Length; i++)
            {
                var value = mHeaders[i];
                if (value == null)
                {
                    continue;
                }

                var key = HeaderNames[i];

                Console.WriteLine(key + ": " + value);
            }

            mLines = lines;

            return true;
        }

        public bool send(Socket sock, byte[] bytes, int index, int length)
        {
            while (length > 0)
            {
                var wrlen = sock.Send(bytes, index, length, SocketFlags.None);
                if (wrlen < length)
                {
                    if (wrlen < 0)
                    {
                        return false;
                    }
                }
                else
                {
                    break;
                }

                index += wrlen;
                length -= wrlen;
            }

            return true;
        }

        public bool send(Socket sock, byte[] bytes, int length)
        {
            return send(sock, bytes, 0, length);
        }

        public bool send(Socket sock, byte[] bytes)
        {
            return send(sock, bytes, bytes.Length);
        }

        public bool writeline(Socket sock, string line)
        {
            var bytes = Encoding.UTF8.GetBytes(line);
            return send(sock, bytes) && writeline(sock);
        }

        public bool writeline(Socket link)
        {
            return send(link, LineEnd);
        }

        public bool write(Socket sock)
        {
            if (!writeline(sock, mFirstLine))
            {
                return false;
            }

            foreach (string line in mLines)
            {
                if (!writeline(sock, line))
                {
                    return false;
                }
            }

            return writeline(sock);
        }

        public TcpClient connect()
        {
            var args = mFirstLine.Split(' ');
            if (args.Length < 3)
            {
                return null;
            }

            var url = new CavanUrl(args[1]);

            Console.WriteLine("url = " + url);

            try
            {
                var link = new TcpClient(url.Host, url.Port);

                mUrl = url;
                mVersion = args[2];
                mMethod = args[0].ToUpper();
                mFirstLine = mMethod + " " + mUrl.Path + " " + mVersion;

                return link;
            }
            catch (Exception)
            {
                return null;
            }
        }

        public bool append(byte[] bytes, int index, int length)
        {
            var size = mLength + length;

            if (size > mContent.Length)
            {
                if (size > CONTENT_MAX)
                {
                    return false;
                }

                var content = new byte[(size * 2 + 7) & (~0x07)];
                Array.Copy(mContent, content, mLength);
                mContent = content;
            }

            Array.Copy(bytes, index, mContent, mLength, length);
            mLength += length;

            return true;
        }

        public bool proxy(Socket isock, Socket osock)
        {
            var bytes = new byte[1024];
            var length = ContentLength;

            if (length < 0)
            {
                var links = new ArrayList();

                links.Add(isock);

                while (true)
                {
                    links.Add(osock);

                    Socket.Select(links, null, null, OVERTIME);

                    if (links.Count == 0)
                    {
                        return false;
                    }

                    foreach (Socket link in links)
                    {
                        if (link == osock)
                        {
                            return true;
                        }

                        length = link.Receive(bytes);
                        if (length <= 0)
                        {
                            return false;
                        }

                        if (!send(osock, bytes, length))
                        {
                            return false;
                        }

                        append(bytes, 0, length);
                    }
                }
            }
            else
            {
                while (length > 0)
                {
                    if (!isock.Poll(OVERTIME, SelectMode.SelectRead))
                    {
                        return false;
                    }

                    int rdlen;

                    if (length < bytes.Length)
                    {
                        rdlen = length;
                    }
                    else
                    {
                        rdlen = bytes.Length;
                    }

                    rdlen = isock.Receive(bytes, rdlen, SocketFlags.None);
                    if (rdlen <= 0)
                    {
                        return false;
                    }

                    if (rdlen > length)
                    {
                        return false;
                    }

                    if (!send(osock, bytes, rdlen))
                    {
                        return false;
                    }

                    append(bytes, 0, rdlen);
                    length -= rdlen;
                }
            }

            return true;
        }

        public override string ToString()
        {
            var builder = new StringBuilder();

            builder.AppendLine(mFirstLine);

            foreach (string line in mLines)
            {
                builder.AppendLine(line);
            }

            builder.AppendLine();

            if (mLength > 0)
            {
                builder.AppendLine(Encoding.UTF8.GetString(mContent, 0, mLength));
            }

            return builder.ToString();
        }
    }

    public class HttpCapturePacketFull
    {
        private HttpCapturePacket mRequest;
        private HttpCapturePacket mResponse;

        public HttpCapturePacket Request
        {
            get
            {
                return mRequest;
            }

            set
            {
                mRequest = value;
            }
        }

        public HttpCapturePacket Response
        {
            get
            {
                return mResponse;
            }

            set
            {
                mResponse = value;
            }
        }

        public override string ToString()
        {
            return mRequest.Url.ToString();
        }
    }

    public class HttpCaptureClient : CavanTcpClient
    {
        public HttpCaptureClient(TcpClient client) : base(client)
        {
        }

        public override bool mainLoop()
        {
            var req = new HttpCapturePacket();

            if (!req.read(mClient.Client))
            {
                return false;
            }

            var link = req.connect();
            if (link == null)
            {
                return false;
            }

            Console.WriteLine("Method: " + req.Method);

            try
            {
                if (req.Method.Equals("CONNECT"))
                {
                }
                else
                {
                    if (!req.write(link.Client))
                    {
                        return false;
                    }

                    if (!req.proxy(mClient.Client, link.Client))
                    {
                        return false;
                    }

                    var rsp = new HttpCapturePacket();
                    if (!rsp.read(link.Client))
                    {
                        return false;
                    }

                    if (!rsp.write(mClient.Client))
                    {
                        return false;
                    }

                    if (!rsp.proxy(link.Client, mClient.Client))
                    {
                        return false;
                    }

                    var form = Form as FormHttpCapture;
                    form.performPacketCaptured(req, rsp);
                }
            }
            catch (Exception)
            {
                return false;
            }
            finally
            {
                link.Close();
            }

            return false;
        }
    }
}
