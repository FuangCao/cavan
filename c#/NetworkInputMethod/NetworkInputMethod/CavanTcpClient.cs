using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
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
                return mTcpClient.GetStream();
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
            if (mTcpClient == null)
            {
                return null;
            }

            return mTcpClient.Client.RemoteEndPoint.ToString();
        }

        public void disconnect()
        {
            try
            {
                mTcpClient.Close();
                mTcpClient = null;
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }
        }
    }
}
