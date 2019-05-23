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
            return new WebProxyClient(mService, conn);
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
        private ArrayList mLines = new ArrayList();
        private string mAction;
        private string mProto;
        private string mHost;
        private string mPath;
        private string mVersion;

        public string Action
        {
            get
            {
                return mAction.ToUpper();
            }

            set
            {
                mAction = value;
            }
        }

        public string Proto
        {
            get
            {
                return mProto.ToLower();
            }

            set
            {
                mProto = value;
            }
        }

        public string Host
        {
            get
            {
                return mHost;
            }

            set
            {
                mHost = value;
            }
        }

        public string Path
        {
            get
            {
                return mPath;
            }

            set
            {
                mPath = value;
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

        public bool read(Stream stream)
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

            mAction = args[0];
            mVersion = args[2];

            var url = args[1];

            var index = url.IndexOf("://");
            if (index > 0)
            {
                mProto = url.Substring(0, index);
                index += 3;
            }
            else
            {
                mProto = "tcp";
                index = 0;
            }

            var path = url.IndexOf('/', index);
            if (path < 0)
            {
                mPath = "/";
                mHost = url.Substring(index);
            }
            else
            {
                mPath = url.Substring(path);
                mHost = url.Substring(index, path - index);
            }

            mLines.Clear();

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

                mLines.Add(line);
            }

            return true;
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
            writeline(stream, mAction + ' ' + mPath + ' ' + mVersion);

            foreach (var line in mLines)
            {
                writeline(stream, line as string);
            }

            writeline(stream);
        }

        public void sendResponse(StreamWriter writer)
        {
            writer.WriteLine("HTTP/1.1 200 Connection Established");
            writer.WriteLine();
            writer.Flush();
        }

        public void sendResponse(NetworkStream stream)
        {
            sendResponse(new StreamWriter(stream));
        }

        public TcpClient connect()
        {
            if (mHost == null)
            {
                return null;
            }

            string host;
            int port;

            var index = mHost.IndexOf(':');
            if (index > 0)
            {
                host = mHost.Substring(0, index);
                port = Convert.ToUInt16(mHost.Substring(index + 1));
            }
            else
            {
                host = mHost;

                if (mProto != null)
                {
                    switch (mProto.ToLower())
                    {
                        case "https":
                            port = 443;
                            break;

                        default:
                            port = 80;
                            break;
                    }
                }
                else
                {
                    port = 80;
                }
            }

            try
            {
                return new TcpClient(host, port);
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

        public WebProxyClient(CavanTcpService service, TcpClient client) : base(service, client)
        {
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
                if (!req.read(mClient.GetStream()))
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

                    if (req.Action == "CONNECT")
                    {
                        req.sendResponse(mClient.GetStream());
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
