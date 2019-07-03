using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net.Sockets;
using System.Net.Security;
using System.Text;
using System.Windows.Forms;
using NetworkInputMethod.Properties;
using System.Collections;
using System.Security.Cryptography.X509Certificates;
using System.IO;

namespace NetworkInputMethod
{
    public partial class FormHttpCapture : FormTcpService
    {
        private Hashtable mCertTable = new Hashtable();
        private CavanTcpService mService;

        public FormHttpCapture()
        {
            mService = new CavanTcpService(this);
            InitializeComponent();
            loadSslCerts();

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

        private void onPacketCaptured(object sender, EventArgs e)
        {
            listBoxUrl.Items.Add(sender);
        }

        public void performPacketCaptured(HttpCapturePacketFull packet)
        {
            var method = new EventHandler(onPacketCaptured);
            Invoke(method, packet);
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

        private void buttonClear_Click(object sender, EventArgs e)
        {
            listBoxUrl.Items.Clear();
        }

        public X509Certificate getSslCert(string hostname)
        {
            return mCertTable[hostname] as X509Certificate;
        }

        public void loadSslCerts()
        {
            foreach (var node in Settings.Default.HttpCaptureCerts)
            {
                var args = node.Split('|');
                if (args.Length < 3)
                {
                    continue;
                }

                try
                {
                    var cert = new X509Certificate2(args[2], args[1]);
                    mCertTable[args[0]] = cert;

                    Console.WriteLine("add cert: " + args[0]);
                }
                catch (Exception err)
                {
                    MessageBox.Show(err.ToString());
                }
            }
        }

        private void buttonCert_Click(object sender, EventArgs e)
        {
            var form = new FormCertManager();

            if (form.ShowDialog() == DialogResult.OK)
            {
                loadSslCerts();
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
        public const int CONTENT_MAX = 1 << 20;

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

        public string readline(Stream stream, byte[] bytes)
        {
            var length = 0;

            while (true)
            {
                var value = stream.ReadByte();

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
                    bytes[length++] = (byte)value;
                }
                else
                {
                    return null;
                }
            }

            return Encoding.UTF8.GetString(bytes, 0, length);
        }

        public bool read(Stream stream)
        {
            var bytes = new byte[4096];
            string line;

            for (int i = 0; i < mHeaders.Length; i++)
            {
                mHeaders[i] = null;
            }

            line = readline(stream, bytes);
            if (line == null)
            {
                return false;
            }

            mFirstLine = line;

            var lines = new ArrayList();

            while (true)
            {
                line = readline(stream, bytes);
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

            mLines = lines;

            return true;
        }

        public void writeline(Stream stream, string line)
        {
            var bytes = Encoding.UTF8.GetBytes(line);
            stream.Write(bytes, 0, bytes.Length);
            writeline(stream);
        }

        public void writeline(Stream stream)
        {
            stream.Write(LineEnd, 0, LineEnd.Length);
        }

        public void write(Stream stream)
        {
            writeline(stream, mFirstLine);

            foreach (string line in mLines)
            {
                writeline(stream, line);
            }

            writeline(stream);

            stream.Flush();
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

        public bool proxy(Socket isock, Stream istream, Socket osock, Stream ostream)
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

                        length = istream.Read(bytes, 0, bytes.Length);
                        if (length <= 0)
                        {
                            return false;
                        }

                        ostream.Write(bytes, 0, length);
                        append(bytes, 0, length);
                    }
                }
            }
            else
            {
                while (length > 0)
                {
                    int rdlen;

                    if (length < bytes.Length)
                    {
                        rdlen = length;
                    }
                    else
                    {
                        rdlen = bytes.Length;
                    }

                    rdlen = istream.Read(bytes, 0, bytes.Length);
                    if (rdlen <= 0)
                    {
                        return false;
                    }

                    ostream.Write(bytes, 0, rdlen);

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

        public void response(Stream stream)
        {
            writeline(stream, "HTTP/1.1 200 Connection Established");
            writeline(stream);
            stream.Flush();
        }
    }

    public class HttpCapturePacketFull
    {
        private CavanUrl mUrl;
        private HttpCapturePacket mRequest;
        private HttpCapturePacket mResponse;

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
            return mUrl.ToString();
        }
    }

    public class HttpCaptureClient : CavanTcpClient
    {
        public HttpCaptureClient(TcpClient client) : base(client)
        {
        }

        private bool CavanSslCertificateValidationCallback(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors)
        {
            return true;
        }

        public override bool mainLoop()
        {
            var req = new HttpCapturePacket();

            if (!req.read(mClient.GetStream()))
            {
                return false;
            }

            var link = req.connect();
            if (link == null)
            {
                return false;
            }

            var url = req.Url;

            Console.WriteLine("Method: " + req.Method);

            try
            {
                if (req.Method.Equals("CONNECT"))
                {
                    req.response(mClient.GetStream());

                    var form = Form as FormHttpCapture;
                    var cert = form.getSslCert(url.Host);

                    if (cert != null)
                    {
                        url.Proto = "https";
                        var sslServer = new SslStream(mClient.GetStream());

                        sslServer.AuthenticateAsServer(cert, false, System.Security.Authentication.SslProtocols.Tls, false);

                        var sslClient = new SslStream(link.GetStream(), false, new RemoteCertificateValidationCallback(CavanSslCertificateValidationCallback), null);
                        sslClient.AuthenticateAsClient(url.Host);

                        while (true)
                        {
                            req = new HttpCapturePacket();

                            if (!req.read(sslServer))
                            {
                                break;
                            }

                            req.write(sslClient);

                            if (!req.proxy(mClient.Client, sslServer, link.Client, sslClient))
                            {
                                return false;
                            }

                            var rsp = new HttpCapturePacket();
                            if (!rsp.read(sslClient))
                            {
                                return false;
                            }

                            rsp.write(sslServer);

                            if (!rsp.proxy(link.Client, sslClient, mClient.Client, sslServer))
                            {
                                return false;
                            }

                            var packet = new HttpCapturePacketFull
                            {
                                Url = url,
                                Request = req,
                                Response = rsp,
                            };

                            form.performPacketCaptured(packet);
                        }
                    }
                    else
                    {
                        TcpProxyClient.ProxyLoop(mClient, link);
                    }
                }
                else
                {
                    req.write(link.GetStream());

                    if (!req.proxy(mClient.Client, mClient.GetStream(), link.Client, link.GetStream()))
                    {
                        return false;
                    }

                    var rsp = new HttpCapturePacket();
                    if (!rsp.read(link.GetStream()))
                    {
                        return false;
                    }

                    rsp.write(mClient.GetStream());

                    if (!rsp.proxy(link.Client, link.GetStream(), mClient.Client, mClient.GetStream()))
                    {
                        return false;
                    }

                    var packet = new HttpCapturePacketFull
                    {
                        Url = url,
                        Request = req,
                        Response = rsp,
                    };

                    var form = Form as FormHttpCapture;
                    form.performPacketCaptured(packet);
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
            finally
            {
                link.Close();
            }

            return false;
        }
    }
}
