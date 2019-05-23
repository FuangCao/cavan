using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;
using System.Collections;

namespace NetworkInputMethod
{
    public partial class FormTcpProxyService : FormTcpService
    {
        private CavanTcpService mService;

        public FormTcpProxyService()
        {
            mService = new CavanTcpService(this);
            InitializeComponent();
        }

        private void FormTcpProxyService_FormClosing(object sender, FormClosingEventArgs e)
        {
            e.Cancel = true;
            Visible = false;
        }

        private void buttonStartStop_Click(object sender, EventArgs e)
        {
            if (mService.Enabled)
            {
                mService.stop(false);
            }
            else
            {
                mService.Port = Convert.ToInt16(textBoxPort.Text);
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
            var args = textBoxUrl.Text.Split(':');
            int port = 80;

            try
            {
                if (args.Length > 1)
                {
                    port = Convert.ToUInt16(args[1]);
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }


            return new TcpProxyClient(mService, conn, args[0], port);
        }

        public override void onTcpClientConnected(object sender, EventArgs e)
        {
            listBoxClients.Items.Add(sender);
        }

        public override void onTcpClientDisconnected(object sender, EventArgs e)
        {
            listBoxClients.Items.Remove(sender);
        }
    }

    public class TcpProxyClient : CavanTcpClient
    {
        public const int SELECT_OVERTIME = 600000000;

        private string mHost;
        private int mPort;

        public TcpProxyClient(CavanTcpService service, TcpClient client, string host, int port) : base(service, client)
        {
            mHost = host;
            mPort = port;
        }

        public static bool ProxyLoop(TcpClient link1, TcpClient link2)
        {
            var bytes = new byte[1024];
            var list = new ArrayList();

            while (true)
            {
                list.Add(link1.Client);
                list.Add(link2.Client);

                Socket.Select(list, null, null, SELECT_OVERTIME);

                foreach (var node in list)
                {
                    NetworkStream istream, ostream;

                    if (node == link1.Client)
                    {
                        istream = link1.GetStream();
                        ostream = link2.GetStream();
                    }
                    else
                    {
                        istream = link2.GetStream();
                        ostream = link1.GetStream();
                    }

                    int length = istream.Read(bytes, 0, bytes.Length);
                    if (length > 0)
                    {
                        ostream.Write(bytes, 0, length);
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
            TcpClient client = new TcpClient();

            try
            {
                client.Connect(mHost, mPort);
                ProxyLoop(mClient, client);
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }
            finally
            {
                client.Close();
            }
        }
    }
}
