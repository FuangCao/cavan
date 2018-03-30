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
        protected TcpClient mTcpClient;

        public CavanTcpClient(TcpClient client)
        {
            mTcpClient = client;
        }

        public TcpClient TcpClient
        {
            get
            {
                return mTcpClient;
            }
        }

        public NetworkStream NetworkStream
        {
            get
            {
                lock (this)
                {
                    if (mTcpClient != null)
                    {
                        return mTcpClient.GetStream();
                    }
                }

                return null;
            }
        }

        public virtual void mainLoop()
        {
            NetworkStream stream = mTcpClient.GetStream();
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
            throw new NotImplementedException();
        }

        public override string ToString()
        {
            lock (this)
            {
                try
                {
                    if (mTcpClient != null)
                    {
                        return mTcpClient.Client.RemoteEndPoint.ToString();
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
                if (mTcpClient != null)
                {
                    mTcpClient.Close();
                    mTcpClient = null;
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
                if (mTcpClient == null)
                {
                    return false;
                }

                try
                {
                    var stream = mTcpClient.GetStream();
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
    }
}
