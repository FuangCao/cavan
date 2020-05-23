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
using NetworkInputMethod.Properties;
using System.Collections.Specialized;

namespace NetworkInputMethod
{
    public partial class FormWebProxyService : FormTcpService
    {
        private CavanTcpService mService;
        private CavanUrl mProxyUrl;
        private TextBox[] mUrls;

        public FormWebProxyService()
        {
            mService = new CavanTcpService(this);
            InitializeComponent();

            mUrls = new TextBox[] { textBoxUrl1, textBoxUrl2, textBoxUrl3 };
            LoadUrls();

            textBoxPort.Text = Settings.Default.WebProxyPort.ToString();
        }

        public void LoadUrls()
        {
            var urls = Settings.Default.WebProxyUrls;

            if (urls != null)
            {
                var index = 0;

                foreach (var url in urls)
                {
                    if (index < mUrls.Length)
                    {
                        mUrls[index].Text = url;
                        index++;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        public void SaveUrls()
        {
            StringCollection urls = new StringCollection();

            for (var i = 0; i < mUrls.Length; i++)
            {
                urls.Add(mUrls[i].Text);
            }

            Settings.Default.WebProxyUrls = urls;
            Settings.Default.Save();
        }

        private void buttonStartStop_Click(object sender, EventArgs e)
        {
            if (mProxyUrl == null)
            {
                mService.toggle(textBoxPort.Text);
            }
            else
            {
                buttonProxy1.ForeColor = Color.Black;
                buttonProxy2.ForeColor = Color.Black;
                buttonProxy3.ForeColor = Color.Black;
                SetProxyUrl(null, null);
            }
        }

        public override void onTcpServiceStarted(object sender, EventArgs e)
        {
            Settings.Default.WebProxyPort = mService.Port;
            buttonStartStop.Text = "停止";
        }

        public override void onTcpServiceStopped(object sender, EventArgs e)
        {
            buttonStartStop.Text = "启动";
        }

        public void SetProxyColor(Button button, Button now)
        {
            if (button == now)
            {
                button.ForeColor = Color.Red;
            }
            else
            {
                button.ForeColor = Color.Black;
            }
        }

        public void SetProxyUrl(Button button, TextBox view)
        {
            if (view == null)
            {
                mProxyUrl = null;
            }
            else
            {
                mProxyUrl = new CavanUrl(view.Text);
            }

            SetProxyColor(buttonProxy1, button);
            SetProxyColor(buttonProxy2, button);
            SetProxyColor(buttonProxy3, button);

            mService.CloseClients();
            SaveUrls();
        }

        public override CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            var url = new CavanUrl(textBoxUrl.Text);
            return new WebProxyClient(conn, url, mProxyUrl);
        }

        public override void onTcpClientConnected(object sender, EventArgs e)
        {
            listBoxClients.Items.Add(sender);
        }

        public override void onTcpClientDisconnected(object sender, EventArgs e)
        {
            listBoxClients.Items.Remove(sender);
        }

        private void FormWebProxyService_Load(object sender, EventArgs e)
        {
            if (Settings.Default.WebProxyEnable)
            {
                buttonStartStop.PerformClick();
            }
        }

        private void buttonProxy1_Click(object sender, EventArgs e)
        {
            SetProxyUrl(buttonProxy1, textBoxUrl1);
        }

        private void buttonProxy2_Click(object sender, EventArgs e)
        {
            SetProxyUrl(buttonProxy2, textBoxUrl2);
        }

        private void buttonProxy3_Click(object sender, EventArgs e)
        {
            SetProxyUrl(buttonProxy3, textBoxUrl3);
        }
    }

    public class CavanHttpRequest
    {
        public static char[] SEPARATOR = { ' ', '\t', '\f' };

        private ArrayList mLines = new ArrayList();
        private CavanUrl mUrl;

        private string mVersion;
        private string mMethod;

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

            mMethod = args[0].ToUpper();
            mVersion = args[2];

            mLines.Clear();

            var path = args[1];

            if (url == null)
            {
                mUrl = new CavanUrl(path);

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

                mLines.Add("");
            }
            else if (path[0] != '/')
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

        public TcpClient connect(TcpClient client, CavanUrl url)
        {
            var host = mUrl.Host;
            var port = mUrl.Port;

            if (client != null)
            {
                if (host.Equals(url.Host) && port == url.Port)
                {
                    return client;
                }

                Console.WriteLine(mUrl + " => " + url);

                client.Close();
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

        private CavanUrl mUrl;
        private CavanUrl mProxy;

        public WebProxyClient(TcpClient client, CavanUrl url, CavanUrl proxy) : base(client)
        {
            mUrl = url;
            mProxy = proxy;
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

        public override bool mainLoop()
        {
            var bytes = new byte[1024];
            TcpClient client = null;
            CavanUrl url = null;

            try
            {
                if (mProxy != null)
                {
                    client = mProxy.Connect();
                    if (client != null)
                    {
                        TcpProxyClient.ProxyLoop(mClient, client);
                    }
                }
                else
                {
                    while (true)
                    {
                        var req = new CavanHttpRequest();
                        if (!req.read(mClient.GetStream(), mUrl))
                        {
                            break;
                        }

                        client = req.connect(client, url);
                        if (client == null)
                        {
                            break;
                        }

                        url = req.Url;

                        var list = new ArrayList();

                        if (req.Method == "CONNECT")
                        {
                            req.discard(mClient.GetStream());
                            req.sendConnResponse(mClient.GetStream());
                            TcpProxyClient.ProxyLoop(mClient, client);
                            return false;
                        }

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
            }
            catch (Exception err)
            {
                Console.WriteLine(err.ToString());
            }
            finally
            {
                if (client != null)
                {
                    client.Close();
                }
            }


            return false;
        }
    }
}
