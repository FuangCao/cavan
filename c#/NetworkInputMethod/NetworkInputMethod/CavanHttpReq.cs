using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
using System.IO;
using System.Net.Sockets;
using System.Net.Security;
using System.Security.Cryptography.X509Certificates;
using System.Threading;

namespace NetworkInputMethod
{
    public class CavanHttpReq : CavanHttpPacket
    {
        private FormHttpSender mForm;
        private TcpClient mTcpClient;
        private string mHost;

        public CavanHttpReq(FormHttpSender form, string host)
        {
            mForm = form;
            mHost = host;
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

        public CavanHttpPacket sendTo(Stream stream)
        {
            if (writeTo(stream))
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

        public void mainLoop(bool https)
        {
            while (true)
            {
                Stream stream;

                if (https)
                {
                    stream = getSslStream();
                }
                else
                {
                    stream = getTcpStream(80);
                }

                if (stream != null)
                {
                    while (true)
                    {
                        CavanHttpPacket rsp = sendTo(stream);
                        if (rsp == null)
                        {
                            break;
                        }

                        byte[] body = rsp.Body;
                        if (body != null)
                        {
                            mForm.WriteLog(Encoding.UTF8.GetString(body));
                        }

                        Thread.Sleep(200);
                    }

                    stream.Close();
                }

                closeTcpClient();

                Thread.Sleep(2000);
            }
        }
    }
}
