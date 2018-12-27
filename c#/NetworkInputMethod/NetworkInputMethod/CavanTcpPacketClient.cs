using System;
using System.Net.Sockets;

namespace NetworkInputMethod
{
    public class CavanTcpPacketClient : CavanTcpClient
    {
        private byte[] mBytes = new byte[32];

        public CavanTcpPacketClient(TcpClient client) : base(client)
        {
        }

        public override void mainLoop()
        {
            NetworkStream stream = mTcpClient.GetStream();
            byte[] header = new byte[2];

            while (fill(stream, header, 0, 2))
            {
                int length = header[0] | header[1] << 8;

                if (length > mBytes.Length)
                {
                    mBytes = new byte[length];
                }

                if (fill(stream, mBytes, 0, length))
                {
                    onDataPacketReceived(mBytes, length);
                }
                else
                {
                    break;
                }
            }
        }

        protected virtual void onDataPacketReceived(byte[] bytes, int length)
        {
            throw new NotImplementedException();
        }

        public override bool send(byte[] bytes, int offset, int length)
        {
            lock (this)
            {
                if (mTcpClient == null)
                {
                    return false;
                }

                byte[] header = { (byte)(length & 0xFF), (byte)(length >> 8) };

                try
                {
                    NetworkStream stream = mTcpClient.GetStream();
                    stream.Write(header, 0, 2);
                    stream.Write(bytes, offset, length);
                }
                catch (Exception)
                {
                    return false;
                }
            }

            return true;
        }
    }
}