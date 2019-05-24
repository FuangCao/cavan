using System;
using System.Text;
using System.Net.Sockets;

namespace NetworkInputMethod
{
    interface CavanTcpClientListener
    {
    }

    public class CavanTcpClient
    {
        protected TcpClient mClient;
        protected CavanTcpService mService;

        public CavanTcpClient(CavanTcpService service, TcpClient client)
        {
            mService = service;
            mClient = client;
        }

        public TcpClient Client
        {
            get
            {
                return mClient;
            }
        }

        public NetworkStream NetworkStream
        {
            get
            {
                lock (this)
                {
                    if (mClient != null)
                    {
                        return mClient.GetStream();
                    }
                }

                return null;
            }
        }

        public virtual void mainLoop()
        {
            NetworkStream stream = mClient.GetStream();
            byte[] bytes = new byte[1024];

            while (true)
            {
                int length = stream.Read(bytes, 0, bytes.Length);
                if (length < 0)
                {
                    break;
                }

                onDataReceived(bytes, length);
            }
        }

        protected virtual void onDataReceived(byte[] bytes, int length)
        {
            mService.onTcpDataReceived(this, bytes, length);
        }

        public override string ToString()
        {
            lock (this)
            {
                try
                {
                    if (mClient != null)
                    {
                        return mClient.Client.RemoteEndPoint.ToString();
                    }
                }
                catch (Exception)
                {
                }
            }

            return "disconnected";
        }

        public void disconnect()
        {
            try
            {
                if (mClient != null)
                {
                    mClient.Close();
                    mClient = null;
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }
        }

        public virtual bool send(byte[] bytes, int offset, int length)
        {
            lock (this)
            {
                if (mClient == null)
                {
                    return false;
                }

                try
                {
                    var stream = mClient.GetStream();
                    stream.Write(bytes, offset, length);
                    stream.Flush();
                }
                catch (Exception)
                {
                    return false;
                }
            }

            return true;
        }

        public bool send(byte[] bytes, int length)
        {
            return send(bytes, 0, length);
        }

        public bool send(byte[] bytes)
        {
            return send(bytes, 0, bytes.Length);
        }

        public bool send(string text)
        {
            byte[] bytes = UTF8Encoding.UTF8.GetBytes(text);
            return send(bytes);
        }

        public static bool fill(NetworkStream stream, byte[] bytes, int offset, int length)
        {
            while (length > 0)
            {
                var rdlen = stream.Read(bytes, offset, length);
                if (rdlen <= 0)
                {
                    return false;
                }

                offset += rdlen;
                length -= rdlen;
            }

            return true;
        }
    }

    public class CavanUrl
    {
        private string mProto;
        private string mHost;
        private UInt16 mPort;
        private string mPath;

        public CavanUrl(string proto, string host, UInt16 port, string path)
        {
            mProto = proto.ToLower();
            mHost = host;
            mPort = port;
            mPath = path;
        }

        public CavanUrl(string host, UInt16 port, string path)
        {
            mProto = "tcp";
            mHost = host;
            mPort = port;
            mPath = path;
        }

        public CavanUrl(string host, UInt16 port)
        {
            mProto = "tcp";
            mHost = host;
            mPort = port;
            mPath = "/";
        }

        public CavanUrl(string url)
        {
            if (string.IsNullOrEmpty(url))
            {
                url = "127.0.0.1:80";
            }

            var index = url.IndexOf("://");

            if (index < 0)
            {
                mProto = "http";
                index = 0;
            }
            else
            {
                mProto = url.Substring(0, index).ToLower();
                index += 3;
            }

            var path = url.IndexOf('/', index);

            if (path < 0)
            {
                mPath = "/";
                path = url.Length;
            }
            else
            {
                mPath = url.Substring(path);
            }

            var port = url.IndexOf(':', index, path - index);

            if (port < 0)
            {
                switch (mProto)
                {
                    case "https":
                        mPort = 443;
                        break;

                    case "ftp":
                        mPort = 21;
                        break;

                    default:
                        mPort = 80;
                        break;
                }

                mHost = url.Substring(index, path - index);
            }
            else
            {
                mHost = url.Substring(index, port - index);

                try
                {
                    port++;
                    mPort = Convert.ToUInt16(url.Substring(port, path - port));
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                    mPort = 80;
                }

            }
        }

        public string Proto
        {
            get
            {
                return mProto;
            }

            set
            {
                mProto = value.ToLower();
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

        public UInt16 Port
        {
            get
            {
                return mPort;
            }

            set
            {
                mPort = value;
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

        public string HostPort
        {
            get
            {
                return mHost + ":" + mPort;
            }
        }

        public override string ToString()
        {
            return mProto + "://" + HostPort + mPath;
        }
    }
}
