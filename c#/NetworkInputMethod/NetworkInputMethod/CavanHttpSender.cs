using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.Net.Security;
using System.Threading;
using System.IO;
using System.Security.Cryptography.X509Certificates;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.Windows.Forms;

namespace NetworkInputMethod
{
    public class CavanHttpSender
    {
        private FormHttpSender mForm;
        private TcpClient mTcpClient;
        private string mHost;
        private Stream mStream;
        private bool mIsHttps = true;
        private CavanThread mSendThread;
        private CavanThread mRecvThread;
        private HashSet<CavanHttpPacket> mPackets = new HashSet<CavanHttpPacket>();

        public CavanHttpSender(FormHttpSender form)
        {
            mForm = form;
            mSendThread = new CavanThread("Send", new ThreadStart(SendMainLoop));
            mRecvThread = new CavanThread("Recv", new ThreadStart(RecvMainLoop));
        }

        public void Start()
        {
            mSendThread.Start();
        }

        public void Stop()
        {
            mSendThread.Stop();
            mRecvThread.Stop();
            closeTcpClient();
        }

        public string Host
        {
            get
            {
                if (mHost == null)
                {
                    string host = null;

                    foreach (CavanHttpPacket packet in mPackets)
                    {
                        if (host == null)
                        {
                            host = packet.getHost();
                        }
                        else if (!host.Equals(packet.getHost()))
                        {
                            return null;
                        }
                    }

                    mHost = host;
                }

                return mHost;
            }

            set
            {
                mHost = value;
            }
        }

        public bool IsHttps
        {
            get
            {
                return mIsHttps;
            }

            set
            {
                mIsHttps = value;
            }
        }

        public bool addPacket(CavanHttpPacket packet)
        {
            return mPackets.Add(packet);
        }

        public bool remotePacket(CavanHttpPacket packet)
        {
            return mPackets.Remove(packet);
        }

        public HashSet<CavanHttpPacket> getPakcets()
        {
            return mPackets;
        }

        public int getPacketCount()
        {
            return mPackets.Count;
        }

        private bool parseFileLocked(string pathname)
        {
            if (pathname == null || pathname.Length == 0)
            {
                MessageBox.Show("请选择请求文件！");
                return false;
            }

            string[] lines;

            try
            {
                lines = File.ReadAllLines(pathname);
            }
            catch
            {
                lines = null;
            }

            if (lines == null)
            {
                MessageBox.Show("无法读取文件: " + pathname);
                return false;
            }

            mPackets.Clear();

            CavanHttpPacket packet = new CavanHttpPacket();

            foreach (string line in lines)
            {
                if (packet.addLine(line))
                {
                    mPackets.Add(packet);
                    packet = new CavanHttpPacket();
                }
            }

            if (packet.addLine(null))
            {
                mPackets.Add(packet);
            }

            int count = mPackets.Count;
            if (count > 0)
            {
                mForm.WriteLog("发现 " + count + " 个请求");
                return true;
            }

            MessageBox.Show("没有发现HTTP请求！");

            return false;
        }

        public bool parseFile(string pathname)
        {
            lock (this)
            {
                return parseFileLocked(pathname);
            }
        }

        public TcpClient openTcpClient(ushort port)
        {
            if (mTcpClient != null)
            {
                if (mTcpClient.Connected)
                {
                    return mTcpClient;
                }

                mTcpClient.Close();
                mTcpClient = null;
            }

            string host = Host;
            if (host == null)
            {
                return null;
            }

            string[] args = host.Split(':');

            if (args.Length > 1)
            {
                host = args[0];
                port = Convert.ToUInt16(args[1]);
            }

            mForm.WriteLog(host + ":" + port);

            try
            {
                mTcpClient = new TcpClient(host, port);
                return mTcpClient;
            }
            catch (ThreadAbortException)
            {
                throw;
            }
            catch
            {
                return null;
            }
        }

        public void closeTcpClient()
        {
            Stream stream = mStream;
            if (stream != null)
            {
                mStream = null;
                stream.Close();
            }

            TcpClient client = mTcpClient;
            if (client != null)
            {
                mTcpClient = null;
                client.Close();
            }

            mSendThread.Enqueue(null);
            mRecvThread.Enqueue(null);
        }

        public NetworkStream getTcpStream(ushort port)
        {
            TcpClient tcp = openTcpClient(port);
            if (tcp == null)
            {
                return null;
            }

            return tcp.GetStream();
        }

        public SslStream getSslStream()
        {
            NetworkStream stream = getTcpStream(443);
            if (stream == null)
            {
                return null;
            }

            SslStream ssl = null;

            try
            {
                ssl = new SslStream(stream, false, new RemoteCertificateValidationCallback(CavanSslCertificateValidationCallback), null);
                ssl.AuthenticateAsClient("CavanSsl");
                return ssl;
            }
            catch (Exception e)
            {
                if (ssl != null)
                {
                    ssl.Close();
                }

                stream.Close();

                closeTcpClient();

                if (e is ThreadAbortException)
                {
                    throw e;
                }

                return null;
            }
        }

        private bool CavanSslCertificateValidationCallback(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors)
        {
            return true;
        }

        private bool processBody(byte[] body)
        {
            string text = Encoding.UTF8.GetString(body);

            if (mForm.isDebugEnabled())
            {
                Console.WriteLine(text);
            }

            try
            {
                JObject json = JsonConvert.DeserializeObject<JObject>(text);
                if (json == null)
                {
                    return false;
                }

                JToken ret = json["ret"];
                if (ret == null || ret.Value<int>() != 0)
                {
                    mForm.WriteLog("非法的请求");
                    return false;
                }

                JToken data = json["data"];
                if (data == null)
                {
                    return false;
                }

                JToken code = data["code"];
                if (code == null)
                {
                    return false;
                }

                if (code.Value<int>() == 0)
                {
                    mForm.WriteLog("领取成功");
                    return false;
                }

                JToken errdesc = data["errdesc"];
                if (errdesc == null)
                {
                    return false;
                }

                string message = errdesc.Value<string>();

                if (message.Contains("领取过") || message.Contains("上限"))
                {
                    mForm.WriteLog("已经领过了");
                    return false;
                }

                if (mForm.isDebugEnabled())
                {
                    mForm.WriteLog(message);
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }

            return true;
        }

        public void SendMainLoop()
        {
            while (mForm.SendEnabled && mPackets.Count > 0)
            {
                Stream stream;

                if (mIsHttps)
                {
                    stream = getSslStream();
                }
                else
                {
                    stream = getTcpStream(80);
                }

                if (stream != null)
                {
                    mStream = stream;

                    mSendThread.Clear();

                    foreach (CavanHttpPacket packet in mPackets)
                    {
                        mSendThread.Enqueue(packet);
                    }

                    mRecvThread.Start();

                    mForm.waitForSend();

                    while (mForm.SendEnabled)
                    {
                        CavanHttpPacket packet = mSendThread.Dequeue();

                        if (packet != null && packet.writeTo(stream))
                        {
                            mRecvThread.Enqueue(packet);
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                closeTcpClient();
                Thread.Sleep(100);
            }
        }

        public void RecvMainLoop()
        {
            CavanHttpPacket response = new CavanHttpPacket();

            while (mForm.SendEnabled)
            {
                Stream stream = mStream;
                if (stream == null)
                {
                    break;
                }

                CavanHttpPacket packet = mRecvThread.Dequeue();

                if (packet != null && response.readFrom(stream))
                {
                    byte[] body = response.Body;
                    if (body != null && processBody(body))
                    {
                        mSendThread.Enqueue(packet);
                    }
                }
                else
                {
                    break;
                }
            }

            closeTcpClient();
        }
    }
}
