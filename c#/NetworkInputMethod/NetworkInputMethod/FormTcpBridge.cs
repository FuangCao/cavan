using NetworkInputMethod.Properties;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace NetworkInputMethod
{
    public partial class FormTcpBridge : CavanSubForm
    {
        public FormTcpBridge()
        {
            InitializeComponent();

            var bridges = Settings.Default.TcpBridges;
            if (bridges != null)
            {
                foreach (var bridge in bridges)
                {
                    var args = bridge.Split('|');
                    if (args.Length != 2)
                    {
                        continue;
                    }

                    addBridgeThread(args[0], args[1]);
                }
            }
        }

        public ListView ListViewBridges
        {
            get
            {
                return listViewBridges;
            }
        }

        private void buttonAdd_Click(object sender, EventArgs e)
        {
            var url1 = textBoxUrl1.Text;
            var url2 = textBoxUrl2.Text;

            var thread = addBridgeThread(url1, url2);
            if (thread == null)
            {
                return;
            }

            var bridges = Settings.Default.TcpBridges;
            if (bridges == null)
            {
                bridges = new System.Collections.Specialized.StringCollection();
                Settings.Default.TcpBridges = bridges;
            }

            bridges.Add(url1 + '|' + url2);
            Settings.Default.Save();

            thread.Start();
        }

        public TcpBridgeThread addBridgeThread(string url1, string url2)
        {
            if (string.IsNullOrEmpty(url1))
            {
                return null;
            }

            if (string.IsNullOrEmpty(url2))
            {
                return null;
            }

            return new TcpBridgeThread(this, url1, url2);
        }

        private void toolStripMenuItemStart_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listViewBridges.SelectedItems)
            {
                var thread = item.Tag as TcpBridgeThread;
                thread.Start();
            }
        }

        private void toolStripMenuItemStop_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listViewBridges.SelectedItems)
            {
                var thread = item.Tag as TcpBridgeThread;
                thread.Stop();
            }
        }

        private void toolStripMenuItemRemove_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listViewBridges.SelectedItems)
            {
                var thread = item.Tag as TcpBridgeThread;
                thread.Stop();

                listViewBridges.Items.Remove(item);
            }

            var bridges = Settings.Default.TcpBridges;
            if (bridges != null)
            {
                bridges.Clear();
            }
            else
            {
                bridges = new StringCollection();
            }

            foreach (ListViewItem item in listViewBridges.Items)
            {
                var sub = item.SubItems;
                bridges.Add(sub[1].Text + '|' + sub[2].Text);
            }

            Settings.Default.TcpBridges = bridges;
            Settings.Default.Save();
        }

        private void FormTcpBridge_Load(object sender, EventArgs e)
        {
            if (Settings.Default.TcpBridgeEnable)
            {
                foreach (ListViewItem item in listViewBridges.Items)
                {
                    var thread = item.Tag as TcpBridgeThread;
                    thread.Start();
                }
            }
        }
    }

    public class TcpBridgeThread
    {
        private FormTcpBridge mForm;
        private ListViewItem mItem;
        private Thread mThread;
        private CavanUrl mUrl1;
        private CavanUrl mUrl2;
        private TcpClient mLink1;
        private TcpClient mLink2;

        public TcpBridgeThread(FormTcpBridge form, string url1, string url2)
        {
            var item = form.ListViewBridges.Items.Add("停止");
            item.SubItems.Add(url1);
            item.SubItems.Add(url2);
            item.Tag = this;
            mItem = item;
            mForm = form;

            mUrl1 = new CavanUrl(url1);
            mUrl2 = new CavanUrl(url2);
        }

        public void Start()
        {
            lock (this)
            {
                if (mThread == null)
                {
                    mThread = new Thread(new ThreadStart(MainLoop));
                    mThread.IsBackground = true;
                    mThread.Start();
                }
                else
                {
                    Monitor.PulseAll(this);
                }
            }
        }

        public void Stop()
        {
            lock (this)
            {
                if (mThread != null)
                {
                    mThread = null;

                    var link = mLink1;
                    if (link != null)
                    {
                        link.Close();
                    }

                    link = mLink2;
                    if (link != null)
                    {
                        link.Close();
                    }
                }

                Monitor.PulseAll(this);
            }
        }

        public void PerformUpdateState(string state)
        {
            mForm.Invoke(new EventHandler(OnStateUpdated), state);
        }

        public void OnStateUpdated(object sender, EventArgs e)
        {
            mItem.SubItems[0].Text = sender.ToString();
        }

        public void MainLoop()
        {
            var thread = mThread;
            int delay = 0;

            if (thread == null)
            {
                return;
            }

            while (true)
            {
                TcpClient link1 = null;
                TcpClient link2 = null;

                try
                {
                    lock (this)
                    {
                        if (thread != mThread)
                        {
                            break;
                        }

                        if (delay > 0)
                        {
                            PerformUpdateState("等待" + delay);
                            Monitor.Wait(this, delay * 1000);
                        }

                        if (delay < 10)
                        {
                            delay++;
                        }

                        if (thread != mThread)
                        {
                            break;
                        }
                    }

                    PerformUpdateState("连接1");

                    link1 = mUrl1.Connect();
                    if (link1 == null)
                    {
                        continue;
                    }

                    lock (this)
                    {
                        if (thread != mThread)
                        {
                            break;
                        }
                    }

                    PerformUpdateState("连接2");

                    link2 = mUrl2.Connect();
                    if (link2 == null)
                    {
                        continue;
                    }

                    lock (this)
                    {
                        if (thread != mThread)
                        {
                            break;
                        }
                    }

                    PerformUpdateState("运行");

                    mLink1 = link1;
                    mLink2 = link2;

                    var time = DateTime.Now;
                    TcpProxyClient.ProxyLoop(link1, link2);

                    var diff = DateTime.Now - time;
                    if (diff.TotalSeconds > 20)
                    {
                        delay = 0;
                    }
                }
                catch (ThreadAbortException)
                {
                    break;
                }
                catch (Exception err)
                {
                    Console.WriteLine(err);
                }
                finally
                {
                    if (link2 != null)
                    {
                        link2.Close();
                    }

                    mLink2 = null;

                    if (link1 != null)
                    {
                        link1.Close();
                    }

                    mLink1 = null;
                }
            }

            PerformUpdateState("停止");
        }
    }
}
