using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Collections;

namespace NetworkInputMethod
{
    public partial class FormWebProxyService : FormTcpService
    {
        private CavanTcpService mService;

        public FormWebProxyService()
        {
            mService = new CavanTcpService(this);
            InitializeComponent();
        }

        private void buttonStartStop_Click(object sender, EventArgs e)
        {
            if (mService.Running)
            {
                mService.stop();
            }
            else
            {
                mService.Port = Convert.ToUInt16(textBoxPort.Text);
                mService.start();
            }
        }

        public override void onTcpServiceStarted(object sender, EventArgs e)
        {
            buttonStartStop.Text = "停止";
        }

        public override void onTcpServiceStopped(object sender, EventArgs e)
        {
            buttonStartStop.Text = "启动";
        }

        public override CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            var url = new CavanUrl(textBoxUrl.Text);
            return new WebProxyClient(mService, conn, url);
        }

        public override void onTcpClientConnected(object sender, EventArgs e)
        {
            listBoxClients.Items.Add(sender);
        }

        public override void onTcpClientDisconnected(object sender, EventArgs e)
        {
            listBoxClients.Items.Remove(sender);
        }

        private void FormWebProxyService_FormClosing(object sender, FormClosingEventArgs e)
        {
            e.Cancel = true;
            Visible = false;
        }
    }

    public class CavanHttpRequest
    {
        static char[] SEPARATOR = { ' ', '\t', '\f' };

        private ArrayList mLines = new ArrayList();
        private CavanUrl mUrl;

        private string mVersion;
        private string mMethod;

        public string Method
        {
            get
            {
                return mMethod.ToUpper();
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

        public string readline(Stream stream)
        {
            var builder = new StringBuilder();

            while (true)
            {
                int value = stream.ReadByte();
                if (value < 0)
                {
                    break;
                }

                if (value == '\r')
                {
                    continue;
                }

                if (value == '\n')
                {
                    break;
                }

                builder.Append((char)value);
            }

            return builder.ToString();
        }

        public bool read(Stream stream, CavanUrl url)
        {
            var line = readline(stream);
            if (line == null)
            {
                return false;
            }

            Console.WriteLine(line);

            var args = line.Split(' ');
            if (args.Length < 3)
            {
                return false;
            }

            var path = args[1];

            mMethod = args[0];
            mVersion = args[2];

            mLines.Clear();

            if (path[0] != '/')
            {
                mUrl = new CavanUrl(path);
            }
            else
            {
                mUrl = new CavanUrl(url.Proto, url.Host, url.Port, path);

                while (true)
                {
                    line = readline(stream);
                    if (line == null)
                    {
                        return false;
                    }

                    if (line.Length == 0)
                    {
                        break;
                    }

                    args = line.Split(new char[] { ':' }, 2);
                    if (args.Length > 1)
                    {
                        var key = args[0].Trim().ToUpper();

                        if (key == "HOST")
                        {
                            line = "Host: " + url.HostPort;
                        }
                    }

                    mLines.Add(line);
                }

                mLines.Add("");
            }

            return true;
        }

        public bool discard(Stream stream)
        {
            var bytes = new byte[1024];
            var count = 0;

            while (true)
            {
                var length = stream.Read(bytes, 0, bytes.Length);
                if (length <= 0)
                {
                    break;
                }

                for (var i = 0; i < length; i++)
                {
                    var value = bytes[i];

                    if (value == '\r')
                    {
                        continue;
                    }

                    if (value == '\n')
                    {
                        if (count == 0)
                        {
                            return true;
                        }

                        count = 0;
                    }
                    else
                    {
                        count++;
                    }
                }
            }

            return false;
        }

        public void write(Stream stream, string text)
        {
            var bytes = Encoding.ASCII.GetBytes(text);
            stream.Write(bytes, 0, bytes.Length);
        }

        public void writeline(Stream stream)
        {
            write(stream, "\r\n");
        }

        public void writeline(Stream stream, string line)
        {
            write(stream, line);
            writeline(stream);
        }

        public void write(NetworkStream stream)
        {
            writeline(stream, mMethod + ' ' + mUrl.Path + ' ' + mVersion);

            foreach (var line in mLines)
            {
                writeline(stream, line as string);
            }
        }

        public void sendConnResponse(StreamWriter writer)
        {
            writer.WriteLine("HTTP/1.1 200 Connection Established");
            writer.WriteLine();
            writer.Flush();
        }

        public void sendConnResponse(NetworkStream stream)
        {
            sendConnResponse(new StreamWriter(stream));
        }

        public TcpClient connect()
        {
            Console.WriteLine(mUrl);

            try
            {
                return new TcpClient(mUrl.Host, mUrl.Port);
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }

            return null;
        }
    }

    public class WebProxyClient : CavanTcpClient
    {
        public const int SELECT_OVERTIME = 60000000;

        private CavanUrl mUrl;

        public WebProxyClient(CavanTcpService service, TcpClient client, CavanUrl url) : base(service, client)
        {
            mUrl = url;
        }

        public static bool ProxyLoop(TcpClient ilink, TcpClient olink)
        {
            var bytes = new byte[1024];
            var list = new ArrayList();

            while (true)
            {
                list.Add(ilink.Client);
                list.Add(olink.Client);

                Socket.Select(list, null, null, SELECT_OVERTIME);

                if (list.Count == 0)
                {
                    return false;
                }

                foreach (var node in list)
                {
                    if (node == olink.Client)
                    {
                        return true;
                    }

                    int length = ilink.GetStream().Read(bytes, 0, bytes.Length);
                    if (length > 0)
                    {
                        olink.GetStream().Write(bytes, 0, length);
                    }
                    else
                    {
                        return false;
                    }
                }

                list.Clear();
            }
        }

        public override void mainLoop()
        {
            var bytes = new byte[1024];

            while (true)
            {
                var req = new CavanHttpRequest();
                if (!req.read(mClient.GetStream(), mUrl))
                {
                    break;
                }

                var client = req.connect();
                if (client == null)
                {
                    break;
                }

                try
                {
                    var list = new ArrayList();

                    if (req.Method == "CONNECT")
                    {
                        req.discard(mClient.GetStream());
                        req.sendConnResponse(mClient.GetStream());
                        TcpProxyClient.ProxyLoop(mClient, client);
                        break;
                    }
                    else
                    {
                        req.write(client.GetStream());

                        if (!ProxyLoop(mClient, client))
                        {
                            break;
                        }

                        if (!ProxyLoop(client, mClient))
                        {
                            break;
                        }
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                    break;
                }
                finally
                {
                    client.Close();
                }
            }
        }
    }
}
