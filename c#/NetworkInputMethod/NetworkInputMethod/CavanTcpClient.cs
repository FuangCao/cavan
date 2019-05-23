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
}
