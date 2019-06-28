using System;
using System.Text;
using System.Net;
using System.Net.Sockets;

namespace NetworkInputMethod
{
    interface CavanTcpClientListener
    {
    }

    public class CavanTcpClient
    {
        protected TcpClient mClient;
        protected CavanTcpServiceBase mService;

        public CavanTcpClient(TcpClient client)
        {
            mClient = client;
        }

        public static TcpClient Connect(string url)
        {
            string host;
            string port;

            var index = url.IndexOf(':');
            if (index > 0)
            {
                host = url.Substring(0, index);
                port = url.Substring(index + 1);
            }
            else
            {
                host = "127.0.0.1";
                port = url;
            }

            try
            {
                return new TcpClient(host, Convert.ToUInt16(port));
            }
            catch (Exception)
            {
                return null;
            }
        }

        public static bool WriteData(NetworkStream stream, byte[] bytes, int index, int length)
        {
            try
            {
                stream.Write(bytes, index, length);
            }
            catch (Exception)
            {
                return false;
            }

            return true;
        }

        public static bool WriteData(NetworkStream stream, byte[] bytes)
        {
            return WriteData(stream, bytes, 0, bytes.Length);
        }

        public static bool WriteData(NetworkStream stream, string text)
        {
            var bytes = Encoding.UTF8.GetBytes(text);
            return WriteData(stream, bytes);
        }

        public static bool WritePacket(NetworkStream stream, byte[] bytes, int index, int length)
        {
            var header = new byte[] { (byte)length, ((byte)(length >> 8)) };
            return WriteData(stream, header) && WriteData(stream, bytes, index, length);
        }

        public static bool WritePacket(NetworkStream stream, byte[] bytes)
        {
            return WritePacket(stream, bytes, 0, bytes.Length);
        }

        public static bool WritePacket(NetworkStream stream, string text)
        {
            var bytes = Encoding.UTF8.GetBytes(text);
            return WritePacket(stream, bytes);
        }

        public static int ReadData(NetworkStream stream, byte[] bytes, int index, int size)
        {
            try
            {
                return stream.Read(bytes, index, size);
            }
            catch (Exception)
            {
                return -1;
            }
        }

        public static int ReadData(NetworkStream stream, byte[] bytes)
        {
            return ReadData(stream, bytes, 0, bytes.Length);
        }

        public static bool Fill(NetworkStream stream, byte[] bytes, int index, int size)
        {
            while (true)
            {
                var length = ReadData(stream, bytes, index, size);
                if (length >= size)
                {
                    return true;
                }

                if (length <= 0)
                {
                    return false;
                }

                index += length;
                size -= length;
            }
        }

        public static bool Fill(NetworkStream stream, byte[] bytes)
        {
            return Fill(stream, bytes, 0, bytes.Length);
        }

        public static int ReadPacket(NetworkStream stream, byte[] bytes, int index, int size)
        {
            var header = new byte[2];

            if (!Fill(stream, header))
            {
                return -1;
            }

            var length = (header[1] & 0xFF) << 8 | (header[0] & 0xFF);
            if (length > 0)
            {
                if (size < length)
                {
                    return -1;
                }

                if (!Fill(stream, bytes, index, length))
                {
                    return -1;
                }
            }

            return length;
        }

        public static int ReadPacket(NetworkStream stream, byte[] bytes)
        {
            return ReadPacket(stream, bytes, 0, bytes.Length);
        }

        public TcpClient Client
        {
            get
            {
                lock (this)
                {
                    return mClient;
                }
            }

            set
            {
                lock (this)
                {
                    mClient = value;
                }
            }
        }

        public CavanTcpServiceBase Service
        {
            get
            {
                lock (this)
                {
                    return mService;
                }
            }

            set
            {
                lock (this)
                {
                    mService = value;
                }
            }
        }

        public TcpClient setClient(TcpClient client)
        {
            lock (this)
            {
                var backup = mClient;
                mClient = client;
                return backup;
            }
        }

        public NetworkStream getStream()
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

        public virtual bool mainLoop()
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

            return false;
        }

        protected virtual void onDataReceived(byte[] bytes, int length)
        {
            mService.onTcpDataReceived(this, bytes, length);
        }

        public IPEndPoint RemoteEndPoint
        {
            get
            {
                var client = mClient;
                if (client != null)
                {
                    return client.Client.RemoteEndPoint as IPEndPoint;
                }

                return null;
            }
        }

        public IPAddress RemoteAddress
        {
            get
            {
                var addr = RemoteEndPoint;
                if (addr == null)
                {
                    return null;
                }

                return addr.Address;
            }
        }

        public int RemotePort
        {
            get
            {
                var addr = RemoteEndPoint;
                if (addr == null)
                {
                    return 0;
                }

                return addr.Port;
            }
        }

        public override string ToString()
        {
            var addr = RemoteEndPoint;
            if (addr != null)
            {
                return addr.ToString();
            }

            return "disconnected";
        }

        public bool connect(string url)
        {
            var client = Connect(url);
            if (client == null)
            {
                return false;
            }

            setClient(client);
            onConnected();

            return true;
        }

        public void disconnect()
        {
            var client = setClient(null);

            if (client != null)
            {
                try
                {
                    client.Close();
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                }

                onDisconnected();
            }
        }

        public void onConnected()
        {
            if (mService != null)
            {
                mService.performTcpClientConnected(this);
            }
        }

        public void onDisconnected()
        {
            if (mService != null)
            {
                mService.performTcpClientDisconnected(this);
            }
        }

        public virtual bool send(NetworkStream stream, byte[] bytes, int offset, int length)
        {
            return WriteData(stream, bytes, offset, length);
        }

        public bool send(byte[] bytes, int offset, int length)
        {
            var stream = getStream();
            if (stream == null)
            {
                return false;
            }

            return send(stream, bytes, offset, length);
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
