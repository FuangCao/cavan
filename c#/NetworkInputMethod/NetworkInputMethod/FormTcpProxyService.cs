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
using NetworkInputMethod.Properties;
using System.Collections.Specialized;

namespace NetworkInputMethod
{
    public partial class FormTcpProxyService : CavanSubForm
    {
        public FormTcpProxyService()
        {
            InitializeComponent();

            var proxys = Settings.Default.TcpProxys;
            if (proxys != null)
            {
                foreach (var proxy in proxys)
                {
                    var args = proxy.Split('|');
                    if (args.Length != 2)
                    {
                        continue;
                    }

                    addTcpProxy(args[0], args[1]);
                }
            }
        }

        public ListView ListViewServices
        {
            get
            {
                return listViewServices;
            }
        }

        private void buttonAdd_Click(object sender, EventArgs e)
        {
            var url = textBoxUrl.Text;
            var port = textBoxPort.Text;

            var service = addTcpProxy(url, port);
            if (service == null)
            {
                return;
            }

            var proxys = Settings.Default.TcpProxys;
            if (proxys == null)
            {
                proxys = new StringCollection();
                Settings.Default.TcpProxys = proxys;
            }

            proxys.Add(url + '|' + port);
            Settings.Default.Save();

            service.start();
        }

        public TcpProxyService addTcpProxy(string url, string port)
        {
            if (string.IsNullOrEmpty(url))
            {
                return null;
            }

            if (string.IsNullOrEmpty(port))
            {
                return null;
            }

            try
            {
                return new TcpProxyService(this, url, Convert.ToInt32(port));
            }
            catch (Exception)
            {
                return null;
            }
        }

        private void toolStripMenuItemStart_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listViewServices.SelectedItems)
            {
                var service = item.Tag as TcpProxyService;
                service.start();
            }
        }

        private void toolStripMenuItemStop_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listViewServices.SelectedItems)
            {
                var service = item.Tag as TcpProxyService;
                service.stop();
            }
        }

        private void toolStripMenuItemRemove_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listViewServices.SelectedItems)
            {
                var service = item.Tag as TcpProxyService;
                service.stop();

                listViewServices.Items.Remove(item);
            }

            var proxys = new StringCollection();

            foreach (ListViewItem item in listViewServices.SelectedItems)
            {
                var sub = item.SubItems;
                proxys.Add(sub[2].Text + '|' + sub[1].Text);
            }

            Settings.Default.TcpProxys = proxys;
            Settings.Default.Save();
        }

        private void FormTcpProxyService_Load(object sender, EventArgs e)
        {
            if (Settings.Default.TcpProxyEnable)
            {
                foreach (ListViewItem item in listViewServices.Items)
                {
                    var service = item.Tag as TcpProxyService;
                    service.start();
                }
            }
        }
    }

    public class TcpProxyService : CavanTcpServiceBase
    {
        private FormTcpProxyService mForm;
        private ListViewItem mItem;
        private CavanUrl mUrl;

        public TcpProxyService(FormTcpProxyService form, string url, int port)
        {
            mUrl = new CavanUrl(url);
            Port = port;

            var item = form.ListViewServices.Items.Add("停止");
            item.Tag = this;

            var sub = item.SubItems;
            sub.Add("0");
            sub.Add(port.ToString());
            sub.Add(url);

            mItem = item;
            mForm = form;
        }

        public void PerformUpdateState(string state)
        {
            mForm.Invoke(new EventHandler(OnStateUpdated), state);
        }

        private void OnStateUpdated(object sender, EventArgs e)
        {
            mItem.SubItems[0].Text = sender.ToString();
        }

        public void PerformUpdateCount()
        {
            mForm.Invoke(new EventHandler(OnCountUpdated));
        }

        private void OnCountUpdated(object sender, EventArgs e)
        {
            mItem.SubItems[1].Text = Count.ToString();
        }

        protected override void onTcpServiceRunning()
        {
            PerformUpdateState("运行");
        }

        protected override void onTcpServiceStarted()
        {
            PerformUpdateState("启动");
        }

        protected override void onTcpServiceStopped()
        {
            PerformUpdateState("停止");
        }

        public override CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            return new TcpProxyClient(conn, mUrl);
        }

        protected override void onTcpClientConnected(CavanTcpClient client)
        {
            PerformUpdateCount();
        }

        protected override void onTcpClientDisconnected(CavanTcpClient client)
        {
            PerformUpdateCount();
        }
    }

    public class TcpProxyClient : CavanTcpClient
    {
        public const int SELECT_OVERTIME = 600000000;

        private CavanUrl mUrl;

        public TcpProxyClient(TcpClient client, CavanUrl url) : base(client)
        {
            mUrl = url;
        }

        public static void ProxyLoop(TcpClient link1, TcpClient link2)
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
                        return;
                    }
                }

                list.Clear();
            }
        }

        public override bool mainLoop()
        {
            TcpClient client = null;

            try
            {
                client = mUrl.Connect();
                if (client != null)
                {
                    ProxyLoop(mClient, client);
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
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
