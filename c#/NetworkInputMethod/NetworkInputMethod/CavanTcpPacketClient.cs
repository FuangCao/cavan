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

            while (true)
            {
                if (stream.Read(header, 0, 2) != 2)
                {
                    break;
                }

                int length = header[0] | header[1] << 8;

                if (length > mBytes.Length)
                {
                    mBytes = new byte[length];
                }

                if (stream.Read(mBytes, 0, length) != length)
                {
                    break;
                }

                onDataPacketReceived(mBytes, length);
            }
        }

        protected virtual void onDataPacketReceived(byte[] bytes, int length)
        {
            throw new NotImplementedException();
        }
    }
}