using System;
using System.Net.Sockets;

namespace NetworkInputMethod
{
    public class CavanTcpPacketClient : CavanTcpClient
    {
        private byte[] mHeader = new byte[2];
        private byte[] mBytes = new byte[32];

        public CavanTcpPacketClient(TcpClient client) : base(client)
        {
        }

        public byte[] Header
        {
            get
            {
                return mHeader;
            }
        }

        public byte[] Bytes
        {
            get
            {
                return mBytes;
            }
        }

        public override bool mainLoop()
        {
            NetworkStream stream = mClient.GetStream();

            while (true)
            {
                int length = read(stream);
                if (length < 0)
                {
                    break;
                }

                onDataPacketReceived(mBytes, length);
            }

            return false;
        }

        protected virtual void onDataPacketReceived(byte[] bytes, int length)
        {
            mService.onTcpPacketReceived(this, bytes, length);
        }

        public int read(NetworkStream stream)
        {
            if (fill(stream, mHeader, 0, 2))
            {
                int length = mHeader[0] | mHeader[1] << 8;

                if (length > mBytes.Length)
                {
                    mBytes = new byte[length];
                }

                if (fill(stream, mBytes, 0, length))
                {
                    return length;
                }
            }

            return -1;
        }

        public override bool send(NetworkStream stream, byte[] bytes, int offset, int length)
        {
            return WritePacket(stream, bytes, offset, length);
        }
    }
}