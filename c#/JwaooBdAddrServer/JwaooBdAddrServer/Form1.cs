using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Windows.Forms;

namespace JwaooBdAddrServer {
    public partial class Form1 : Form {

        public const int MIN_DAEMON_COUNT = 20;
        public const int MAX_DAEMON_COUNT = 20000;
        public const String INDEX_FILE_PATH = "index.dat";

        public static Regex sReqRegex = new Regex("AllocBdAddr:\\s*(\\d+)");

        private UInt32 mIndex;
        private byte[] mBdAddr;
        private int mDaemonIdle;
        private int mDaemonCount;
        private TcpListener mListener;

        delegate void UpdateIndexViewCallback();
        delegate void UpdateServiceStateCallback();
        delegate void UpdateListViewCallback(ListViewItem item, bool remove);

        public Form1() {
            InitializeComponent();

            readIndex();
            updateIndexView();

            updateIpAddress();
            updateServiceState();
        }

        public bool readIndex() {
            FileStream stream;
            try {
                stream = new FileStream(INDEX_FILE_PATH, FileMode.Open);
            } catch {
                return false;
            }

            try {
                byte[] bytes = new byte[4];
                int length = stream.Read(bytes, 0, bytes.Length);
                if (length != bytes.Length) {
                    return false;
                }

                setIndex(BitConverter.ToUInt32(bytes, 0));
            } catch {
                return false;
            } finally {
                stream.Close();
            }

            return true;
        }

        public bool saveIndex(UInt32 index) {
            FileStream stream;
            try {
                stream = new FileStream(INDEX_FILE_PATH, FileMode.OpenOrCreate);
            } catch {
                return false;
            }

            try {
                byte[] bytes = BitConverter.GetBytes(index);
                stream.Write(bytes, 0, bytes.Length);
                stream.Flush();
            } catch {
                return false;
            } finally {
                stream.Close();
            }

            return true;
        }

        public IPAddress getHostIpAddress() {
            IPAddress[] addresses = Dns.GetHostAddresses(Dns.GetHostName());
            if (addresses == null) {
                return null;
            }

            foreach (IPAddress address in addresses) {
                if (address.AddressFamily == AddressFamily.InterNetwork) {
                    return address;
                }
            }

            return null;
        }

        public String getHostIpAddressText() {
            IPAddress address = getHostIpAddress();
            if (address == null) {
                return "-";
            }

            return address.ToString();
        }

        private void updateIpAddress() {
            textBoxIp.Text = getHostIpAddressText();
        }

        private void updateServiceState() {
            if (labelState.InvokeRequired) {
                UpdateServiceStateCallback callback = new UpdateServiceStateCallback(updateServiceState);
                Invoke(callback);
            } else if (mListener != null) {
                textBoxPort.ReadOnly = true;
                buttonStart.Enabled = false;
                buttonStop.Enabled = true;
                labelState.Text = "服务器正在运行，线程数：" + (mDaemonCount - mDaemonIdle) + "/" + mDaemonCount;
            } else {
                textBoxPort.ReadOnly = false;
                buttonStart.Enabled = true;
                buttonStop.Enabled = false;
                labelState.Text = "服务器已停止运行";
            }
        }

        private void updateBdAddr(byte[] addr, UInt32 index) {
            byte[] bytes = BitConverter.GetBytes(index);

            for (int i = bytes.Length - 1, j = 2; i >= 0; i--, j++) {
                addr[j] = bytes[i];
            }
        }

        private void updateBdAddr() {
            updateBdAddr(mBdAddr, mIndex);
        }

        private byte[] getBdAddr(UInt32 index) {
            byte[] addr = new byte[6];

            addr[0] = 0x80;
            addr[1] = 0xEA;

            updateBdAddr(addr, index);

            return addr;
        }

        private byte[] getBdAddr() {
            if (mBdAddr == null) {
                mBdAddr = getBdAddr(mIndex);
            }

            return mBdAddr;
        }

        private String getBdAddrString(byte[] bdAddr) {
            return String.Format("{0:x2}:{1:x2}:{2:x2}:{3:x2}:{4:x2}:{5:x2}",
                bdAddr[0], bdAddr[1], bdAddr[2], bdAddr[3], bdAddr[4], bdAddr[5]);
        }

        private String getBdAddrString() {
            return getBdAddrString(getBdAddr());
        }

        private void setIndex(UInt32 index) {
            mIndex = index;

            if (mBdAddr == null) {
                mBdAddr = getBdAddr(mIndex);
            } else {
                updateBdAddr();
            }

            updateIndexView();
        }

        private void updateIndexView() {
            if (labelNextAddress.InvokeRequired) {
                UpdateIndexViewCallback callback = new UpdateIndexViewCallback(updateIndexView);
                Invoke(callback);
            } else {
                labelNextAddress.Text = getBdAddrString();
            }
        }

        private void updateListView(ListViewItem item, bool remove) {
            if (listViewClients.InvokeRequired) {
                UpdateListViewCallback callback = new UpdateListViewCallback(updateListView);
                Invoke(callback, new Object[] { item, remove });
            } else if (remove) {
                listViewClients.Items.Remove(item);
            } else {
                listViewClients.Items.Add(item);
            }
        }

        private bool ServiceThreadHander(TcpClient client) {
            NetworkStream stream = null;

            try {
                stream = client.GetStream();
                if (stream == null) {
                    return false;
                }

                byte[] bytes = Encoding.Default.GetBytes("JwaooBdAddrServer");
                stream.Write(bytes, 0, bytes.Length);

                bytes = new byte[1024];
                int length = stream.Read(bytes, 0, bytes.Length);
                if (length < 0) {
                    return false;
                }

                String text = System.Text.Encoding.Default.GetString(bytes, 0, length);
                Match match = sReqRegex.Match(text);
                if (match == null || match.Success == false) {
                    return false;
                }

                UInt32 count = UInt32.Parse(match.Groups[1].Value);
                if (count == 0) {
                    return true;
                }

                lock (this) {
                    UInt32 index = mIndex + count;

                    bytes = BitConverter.GetBytes(mIndex);
                    stream.Write(bytes, 0, bytes.Length);

                    if (saveIndex(index)) {
                        setIndex(index);
                    }
                }
            } catch {
                return false;
            } finally {
                if (stream != null) {
                    stream.Close();
                }
            }

            return true;
        }

        private void ServiceThreadHander() {
            lock (this) {
                mDaemonCount++;
            }

            int errCount = 0;

            try {
                while (mListener != null && errCount < 10) {
                    lock (this) {
                        mDaemonIdle++;
                    }

                    updateServiceState();

                    TcpClient client = mListener.AcceptTcpClient();

                    lock (this) {
                        mDaemonIdle--;
                    }

                    updateServiceState();

                    if (client == null) {
                        errCount++;
                        continue;
                    }

                    errCount = 0;

                    lock (this) {
                        if (mDaemonIdle < 1 && mDaemonCount < MAX_DAEMON_COUNT) {
                            Thread thread = new Thread(new ThreadStart(ServiceThreadHander));
                            thread.Start();
                        }
                    }

                    ListViewItem item = new ListViewItem();
                    item.Text = client.Client.RemoteEndPoint.ToString();

                    updateListView(item, false);

                    ServiceThreadHander(client);
                    client.Close();

                    Thread.Sleep(2000);
                    updateListView(item, true);

                    lock (this) {
                        if (mDaemonCount > MIN_DAEMON_COUNT) {
                            break;
                        }
                    }
                }
            } catch (Exception e) {
                if (mListener != null) {
                    MessageBox.Show(e.ToString());
                }
            }

            lock (this) {
                if (--mDaemonCount <= 0) {
                    if (mListener != null) {
                        mListener.Stop();
                        mListener = null;
                    }
                }

                updateServiceState();
            }
        }

        private void Form1_Load(object sender, EventArgs e) {
        }

        private void buttonStart_Click(object sender, EventArgs e) {
            int port;
      
            try {
                port = int.Parse(textBoxPort.Text);
            } catch {
                MessageBox.Show("请输入正确的端口号");
                return;
            }

            try {
                mListener = new TcpListener(IPAddress.Any, port);
                mListener.Start();
            } catch {
                mListener = null;
                MessageBox.Show("无法启动服务, 可能端口号被占用");
                return;
            }

            mDaemonIdle = 0;
            mDaemonCount = 0;

            Thread thread = new Thread(new ThreadStart(ServiceThreadHander));
            thread.Start();
        }

        private void buttonStop_Click(object sender, EventArgs e) {
            if (mListener != null) {
                TcpListener listener = mListener;

                mListener = null;
                listener.Stop();
            }
        }

        private void textBoxIp_Click(object sender, EventArgs e) {
            updateIpAddress();
        }
    }
}
