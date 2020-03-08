using System;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Windows.Forms;

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

        public FormTcpService Form
        {
            get
            {
                return mService.Form;
            }
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
                try
                {
                    if (mClient != null)
                    {
                        return mClient.GetStream();
                    }
                }
                catch (Exception)
                {
                    return null;
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

        public virtual void onConnected()
        {
            if (mService != null)
            {
                mService.performTcpClientConnected(this);
            }
        }

        public virtual void onDisconnected()
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
        private ushort mPort;
        private string mPath;
        private string mUser;
        private string mPass;

        public static TcpClient Connect(string url)
        {
            return new CavanUrl(url).Connect();
        }

        public CavanUrl(string proto, string host, ushort port, string path)
        {
            mProto = proto.ToLower();
            mHost = host;
            mPort = port;
            mPath = path;
        }

        public CavanUrl(string host, ushort port, string path)
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

            var host = url.IndexOf('@', index, path - index);
            if (host > 0)
            {
                var pass = url.IndexOf(':', index, host - index);
                if (pass > 0)
                {
                    mPass = url.Substring(pass + 1, host - pass - 1);
                    mUser = url.Substring(index, pass - index);
                }
                else
                {
                    mUser = url.Substring(index, host - index);
                }

                index = host + 1;
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

                    case "ssh":
                    case "sftp":
                        mPort = 22;
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

        public ushort Port
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

        public string User
        {
            get
            {
                return mUser;
            }

            set
            {
                mUser = value;
            }
        }

        public string Pass
        {
            get
            {
                return mPass;
            }

            set
            {
                mPass = value;
            }
        }

        public string UserPass
        {
            get
            {
                if (mUser != null)
                {
                    if (mPass != null)
                    {
                        return mUser + ":" + mPass;
                    }

                    return mUser;
                }

                if (mPass != null)
                {
                    return mPass;
                }

                return null;
            }
        }

        public string HostPort
        {
            get
            {
                switch (mPort)
                {
                    case 80:
                        if ("http".Equals(mProto))
                        {
                            return mHost;
                        }
                        break;

                    case 443:
                        if ("https".Equals(mProto))
                        {
                            return mHost;
                        }
                        break;
                }

                return mHost + ":" + mPort;
            }
        }

        public TcpClient Connect()
        {
            switch (mProto.ToLower())
            {
                case "adb":
                    return CavanAdbClient.ConnectRemote(mHost, mPort, false);

                default:
                    try
                    {
                        return new TcpClient(mHost, mPort);
                    }
                    catch (Exception err)
                    {
                        Console.WriteLine(err);
                        break;
                    }
            }

            return null;
        }

        public override string ToString()
        {
            var builder = new StringBuilder();

            builder.Append(mProto).Append("://");

            var value = UserPass;
            if (value != null)
            {
                builder.Append(value).Append('@');
            }

            builder.Append(HostPort).Append(mPath);

            return builder.ToString();
        }
    }
}
