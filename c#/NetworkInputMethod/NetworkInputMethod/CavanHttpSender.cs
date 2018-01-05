using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.Net.Security;
using System.Threading;
using System.IO;
using System.Security.Cryptography.X509Certificates;

namespace NetworkInputMethod
{
    public class CavanHttpSender : CavanHttpPacket
    {
        private Thread mThread;
        private FormHttpSender mForm;
        private TcpClient mTcpClient;
        private string mHost;
        private bool mIsHttps = true;

        public CavanHttpSender(FormHttpSender form)
        {
            mForm = form;
        }

        public void start()
        {
            Monitor.Enter(this);

            if (mThread == null)
            {
                mThread = new Thread(new ThreadStart(SendThreadCallback));
                mThread.Start();
            }

            Monitor.Exit(this);
        }

        public void stop()
        {
            Monitor.Enter(this);

            if (mThread != null)
            {
                mThread.Abort();
            }

            Monitor.Exit(this);

            closeTcpClient();
        }

        public string Host
        {
            get
            {
                if (mHost == null)
                {
                    mHost = getHost();
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

        public CavanHttpPacket sendTo(Stream stream, ByteArrayWriter writer)
        {
            if (writeTo(stream, writer))
            {
                CavanHttpPacket rsp = new CavanHttpPacket();
                if (rsp.readFrom(stream))
                {
                    return rsp;
                }
            }

            return null;
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

            mForm.WriteLog("host = " + host);
            mForm.WriteLog("port = " + port);

            try
            {
                mTcpClient = new TcpClient(host, port);
                return mTcpClient;
            }
            catch (ThreadAbortException)
            {
                throw;
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
                return null;
            }
        }

        public void closeTcpClient()
        {
            if (mTcpClient != null)
            {
                mTcpClient.Close();
                mTcpClient = null;
            }
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

                Console.WriteLine(e);

                return null;
            }
        }

        private bool CavanSslCertificateValidationCallback(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors)
        {
            return true;
        }

        public void mainLoop()
        {
            ByteArrayWriter writer = buildBytes();
            int delay = 100;

            while (true)
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
                    mForm.waitForSend();

                    while (true)
                    {
                        CavanHttpPacket rsp = sendTo(stream, writer);
                        if (rsp == null)
                        {
                            break;
                        }

                        byte[] body = rsp.Body;
                        if (body != null)
                        {
                            Console.WriteLine(Encoding.UTF8.GetString(body));
                        }

                        if (rsp.isNeedClose())
                        {
                            break;
                        }

                        Thread.Sleep(delay);
                        delay += 100;
                    }

                    stream.Close();
                }

                closeTcpClient();

                Thread.Sleep(100);
            }
        }

        private void SendThreadCallback()
        {
            try
            {
                Console.WriteLine("started");
                mainLoop();
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception: " + e.GetType());
            }
            finally
            {
                mThread = null;
                closeTcpClient();
                Console.WriteLine("stopped");
            }
        }
    }
}
