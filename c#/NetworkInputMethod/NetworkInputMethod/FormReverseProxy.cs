using System;
using System.Net;
using System.Threading;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Windows.Forms;
using NetworkInputMethod.Properties;

namespace NetworkInputMethod
{
    public partial class FormReverseProxy : FormTcpService
    {
        private Hashtable mProxyLinks = new Hashtable();
        private CavanTcpService mService;

        public FormReverseProxy()
        {
            InitializeComponent();
            textBoxPort.Text = Settings.Default.ReverseProxyPort.ToString();
            mService = new CavanTcpService(this);
        }

        public string LocalUrl
        {
            get
            {
                return mService.Url;
            }
        }

        private void buttonSwitch_Click(object sender, EventArgs e)
        {
            mService.toggle(textBoxPort.Text);
        }

        public override void onTcpServiceStarted(object sender, EventArgs e)
        {
            Settings.Default.ReverseProxyPort = mService.Port;
            buttonSwitch.Text = "停止";
        }

        public override void onTcpServiceStopped(object sender, EventArgs e)
        {
            buttonSwitch.Text = "启动";
        }

        public override CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            return new ReverseProxyLink(conn);
        }

        public void onReverseServiceStarted(object sender, EventArgs e)
        {
            var service = sender as ReverseProxyService;
            service.Item.SubItems[0].Text = "启动";
        }

        public void onReverseServiceRunning(object sender, EventArgs e)
        {
            var service = sender as ReverseProxyService;
            service.Item.SubItems[0].Text = "运行";
            service.Item.SubItems[2].Text = service.Port.ToString();
        }

        public void onReverseServiceStopped(object sender, EventArgs e)
        {
            var service = sender as ReverseProxyService;
            service.Item.SubItems[0].Text = "停止";
        }

        public void onReverseClientConnected(object sender, EventArgs e)
        {
            var client = sender as ReverseProxyClient;
            var service = client.Service as ReverseProxyService;
            service.Item.SubItems[1].Text = client.Service.Count.ToString();
        }

        public void onReverseClientDisconnected(object sender, EventArgs e)
        {
            var client = sender as ReverseProxyClient;
            var service = client.Service as ReverseProxyService;
            service.Item.SubItems[1].Text = client.Service.Count.ToString();
        }

        public string getProxyLinkKey(object address, object hostname)
        {
            return address + "@" + hostname;
        }

        public string getProxyLinkKey(ReverseProxyLink link)
        {
            var address = link.RemoteAddress;
            var hostname = link.Hostname;

            if (address != null)
            {
                if (hostname != null)
                {
                    return getProxyLinkKey(address, hostname);
                }

                return address.ToString();
            }

            if (hostname != null)
            {
                return hostname;
            }

            return "invalid";
        }

        public ListViewItem findProxyLinkItem(string key)
        {
            lock (mProxyLinks)
            {
                return mProxyLinks[key] as ListViewItem;
            }
        }

        public void onProxyLinkCountChanged(object sender, EventArgs e)
        {
            var item = sender as ListViewItem;
            var links = item.Tag as HashSet<ReverseProxyLink>;
            item.SubItems[0].Text = links.Count.ToString();
        }

        public override void onTcpClientDisconnected(object sender, EventArgs e)
        {
            var link = sender as ReverseProxyLink;
            var item = link.Item;

            if (item != null)
            {
                var links = item.Tag as HashSet<ReverseProxyLink>;

                lock (links)
                {
                    links.Remove(link);
                }
            }
        }

        public override void onTcpClientUpdated(object sender, EventArgs e)
        {
            var link = sender as ReverseProxyLink;
            var address = link.RemoteAddress;
            var hostname = link.Hostname;

            if (address == null || hostname == null)
            {
                return;
            }

            var key = getProxyLinkKey(address, hostname);

            var item = findProxyLinkItem(key);
            if (item == null)
            {
                item = listViewClients.Items.Add("0");
                item.SubItems.Add(address.ToString());
                item.SubItems.Add(hostname);
                item.Tag = new HashSet<ReverseProxyLink>();

                lock (mProxyLinks)
                {
                    mProxyLinks[key] = item;
                }
            }

            link.Item = item;

            var links = item.Tag as HashSet<ReverseProxyLink>;

            lock (links)
            {
                links.Add(link);
            }

            item.SubItems[0].Text = links.Count.ToString();
        }

        private void toolStripMenuItemAddProxy_Click(object sender, EventArgs e)
        {
            var form = new FormAddProxy
            {
                ServerUrl = mService.Url
            };

            var items = listViewClients.SelectedItems;
            if (items != null && items.Count > 0)
            {
                var links = items[0].Tag as HashSet<ReverseProxyLink>;
                var link = links.First();

                form.ClientName = link.Hostname;
                form.ClientAddress = link.RemoteAddress.ToString();
            }

            if (form.ShowDialog() == DialogResult.OK)
            {

                var service = new ReverseProxyService
                {
                    Form = this,
                    Port = form.Port,
                    ServerUrl = form.ServerUrl,
                    TargetUrl = form.TargetUrl,
                    ClientAddress = form.ClientAddress,
                    ClientName = form.ClientName
                };

                service.addToListView(listViewProxys);
                service.start();
            }
        }

        private void FormReverseProxy_Load(object sender, EventArgs e)
        {
            if (Settings.Default.ReverseProxyEnable)
            {
                buttonSwitch.PerformClick();
            }
        }

        private void toolStripMenuItemProxyStart_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listViewProxys.SelectedItems)
            {
                var service = item.Tag as ReverseProxyService;
                service.start();
            }
        }

        private void toolStripMenuItemProxyStop_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listViewProxys.SelectedItems)
            {
                var service = item.Tag as ReverseProxyService;
                service.stop();
            }
        }

        private void toolStripMenuItemProxyAdd_Click(object sender, EventArgs e)
        {
            var form = new FormAddProxy
            {
                ServerUrl = mService.Url
            };

            if (form.ShowDialog() == DialogResult.OK)
            {
                var service = new ReverseProxyService
                {
                    Form = this,
                    Port = form.Port,
                    TargetUrl = form.TargetUrl,
                    ServerUrl = form.ServerUrl,
                    ClientAddress = form.ClientAddress,
                    ClientName = form.ClientName
                };

                service.addToListView(listViewProxys);
                service.start();
            }
        }

        private void toolStripMenuItemProxyRemove_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listViewProxys.SelectedItems)
            {
                var service = item.Tag as ReverseProxyService;
                service.stop();
                listViewProxys.Items.Remove(item);
            }
        }

        public ReverseProxyLink getProxyLinkRaw(string key, string url, CavanTcpClient peer)
        {
            var item = findProxyLinkItem(key);
            if (item == null)
            {
                return null;
            }

            var links = item.Tag as HashSet<ReverseProxyLink>;
            if (links.Count == 0)
            {
                return null;
            }

            ReverseProxyLink link = null;

            lock (links)
            {
                foreach (var node in links)
                {
                    Console.WriteLine("node = " + node + ", keepalive = " + node.KeepAlive);

                    if (node.KeepAlive == 0)
                    {
                        link = node;
                        break;
                    }
                }

                if (link == null)
                {
                    return null;
                }

                links.Remove(link);
            }

            var handler = new EventHandler(onProxyLinkCountChanged);
            Invoke(handler, item);

            if (link.sendLinkCommand(url, peer))
            {
                return link;
            }

            link.disconnect();

            return null;
        }

        public ReverseProxyLink getProxyLink(string key, string url, CavanTcpClient peer)
        {
            for (int i = 0; i < 25; i++)
            {
                var link = getProxyLinkRaw(key, url, peer);
                if (link != null)
                {
                    return link;
                }

                Thread.Sleep(200);
            }

            return null;
        }

        public ReverseProxyLink getProxyLink(string address, string name, string url, CavanTcpClient peer)
        {
            var key = getProxyLinkKey(address, name);
            return getProxyLink(key, url, peer);
        }

        private void timerKeepAlive_Tick(object sender, EventArgs e)
        {
            lock (mProxyLinks)
            {
                foreach (ListViewItem item in mProxyLinks.Values)
                {
                    var links = item.Tag as HashSet<ReverseProxyLink>;
                    foreach (ReverseProxyLink link in links)
                    {
                        if (!link.doKeepAlive())
                        {
                            link.disconnect();
                        }
                    }
                }
            }
        }
    }

    public class ReverseProxyLink : CavanTcpPacketClient
    {
        public static char[] SEPARATOR = { ' ', '\t', '\f' };

        private CavanTcpClient mPeer;
        private ListViewItem mItem;
        private string mHostname;
        private int mKeepAlive;

        public ReverseProxyLink(TcpClient client) : base(client)
        {
        }

        public string Hostname
        {
            get
            {
                return mHostname;
            }

            set
            {
                mHostname = value;
            }
        }

        public ListViewItem Item
        {
            get
            {
                return mItem;
            }

            set
            {
                mItem = value;
            }
        }

        public int KeepAlive
        {
            get
            {
                return mKeepAlive;
            }

            set
            {
                mKeepAlive = value;
            }
        }

        public CavanTcpClient Peer
        {
            get
            {
                return mPeer;
            }

            set
            {
                mPeer = value;
            }
        }

        public override bool mainLoop()
        {
            var stream = getStream();
            if (stream == null)
            {
                return false;
            }

            while (true)
            {
                var length = read(stream);
                if (length < 0)
                {
                    break;
                }

                var args = Encoding.ASCII.GetString(Bytes, 0, length).Split(SEPARATOR);
                var command = args[0];

                switch (command)
                {
                    case "ping":
                        send("pong");
                        break;

                    case "pong":
                        mKeepAlive = 0;
                        break;

                    case "login":
                        mHostname = args[1];
                        mService.onTcpClientUpdated(this);
                        break;

                    case "link":
                        return processCommandLink(args);

                    case "linked":
                        if (processCommandLinked(args))
                        {
                            if (mPeer != null)
                            {
                                mPeer.disconnect();
                            }

                            return false;
                        }
                        break;
                }
            }

            return false;
        }

        private bool processCommandLinked(string[] args)
        {
            var peer = mPeer;
            if (peer == null)
            {
                return false;
            }

            if (args.Length > 1)
            {
                try
                {
                    var error = Convert.ToInt32(args[1]);
                    if (error != 0)
                    {
                        return false;
                    }
                }
                catch (Exception)
                {
                    return true;
                }
            }

            TcpProxyClient.ProxyLoop(Client, peer.Client);

            return true;
        }

        private bool processCommandLink(string[] args)
        {
            if (args.Length > 2)
            {
                var service = mService as CavanTcpService;
                var form = service.Form as FormReverseProxy;
                var link = form.getProxyLink(args[1], args[2], this);
                if (link == null)
                {
                    return false;
                }

                return true;
            }

            if (args.Length > 1)
            {
                var link = Connect(args[1]);
                if (link == null)
                {
                    return false;
                }

                try
                {
                    TcpProxyClient.ProxyLoop(link, Client);
                }
                catch (Exception)
                {
                }
                finally
                {
                    link.Close();
                }
            }

            return false;
        }

        public bool doKeepAlive()
        {
            if (mKeepAlive > 0)
            {
                return mKeepAlive < 5;
            }

            mKeepAlive = 1;

            return send("ping");
        }

        internal bool sendLinkCommand(string url, CavanTcpClient peer)
        {
            if (send("link2 " + url))
            {
                mPeer = peer;
                return true;
            }

            return false;
        }
    }

    public class ReverseProxyService : CavanTcpServiceBase
    {
        private FormReverseProxy mForm;
        private string mClientAddress;
        private string mClientName;
        private string mServerUrl;
        private string mTargetUrl;
        private ListViewItem mItem;

        public FormReverseProxy Form
        {
            get
            {
                return mForm;
            }

            set
            {
                mForm = value;
            }
        }

        public string ClientAddress
        {
            get
            {
                return mClientAddress;
            }

            set
            {
                mClientAddress = value;
            }
        }

        public string ClientName
        {
            get
            {
                return mClientName;
            }

            set
            {
                mClientName = value;
            }
        }

        public string Key
        {
            get
            {
                return mForm.getProxyLinkKey(mClientAddress, mClientName);
            }
        }

        public string ServerUrl
        {
            get
            {
                return mServerUrl;
            }

            set
            {
                mServerUrl = value;
            }
        }

        public string TargetUrl
        {
            get
            {
                return mTargetUrl;
            }

            set
            {
                mTargetUrl = value;
            }
        }

        public ListViewItem Item
        {
            get
            {
                return mItem;
            }

            set
            {
                mItem = value;
            }
        }

        protected override void onTcpServiceStarted()
        {
            EventHandler handler = new EventHandler(mForm.onReverseServiceStarted);
            mForm.Invoke(handler, this, null);
        }

        protected override void onTcpServiceRunning()
        {
            EventHandler handler = new EventHandler(mForm.onReverseServiceRunning);
            mForm.Invoke(handler, this, null);
        }

        protected override void onTcpServiceStopped()
        {
            EventHandler handler = new EventHandler(mForm.onReverseServiceStopped);
            mForm.Invoke(handler, this, null);
        }

        protected override void onTcpClientConnected(CavanTcpClient client)
        {
            EventHandler handler = new EventHandler(mForm.onReverseClientConnected);
            mForm.Invoke(handler, client, null);
        }

        protected override void onTcpClientDisconnected(CavanTcpClient client)
        {
            EventHandler handler = new EventHandler(mForm.onReverseClientDisconnected);
            mForm.Invoke(handler, client, null);
        }

        public override CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            return new ReverseProxyClient(conn);
        }

        public void addToListView(ListView view)
        {
            var item = view.Items.Add("停止");
            var sub = item.SubItems;

            sub.Add("0");
            sub.Add(Port.ToString());
            sub.Add(TargetUrl);
            sub.Add(ServerUrl);
            sub.Add(ClientAddress);
            sub.Add(ClientName);

            item.Tag = this;
            mItem = item;
        }

        public static explicit operator ReverseProxyService(CavanTcpService service)
        {
            return (ReverseProxyService)service;
        }

        public bool proxyLoop(CavanTcpClient peer)
        {
            if (ServerUrl.Equals(mForm.LocalUrl))
            {
                var link = mForm.getProxyLink(mClientAddress, mClientName, TargetUrl, peer);
                if (link == null)
                {
                    return false;
                }

                return true;
            }

            var client = CavanTcpClient.Connect(ServerUrl);
            if (client == null)
            {
                return false;
            }

            if (CavanTcpClient.WritePacket(client.GetStream(), "link " + Key + " " + TargetUrl)) ;
            {
                TcpProxyClient.ProxyLoop(client, peer.Client);
            }

            return false;
        }
    }

    public class ReverseProxyClient : CavanTcpClient
    {
        public ReverseProxyClient(TcpClient client) : base(client)
        {
        }

        public override bool mainLoop()
        {
            var service = (ReverseProxyService)mService;
            return service.proxyLoop(this);
        }
    }
}
