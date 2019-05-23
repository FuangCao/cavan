using System;
using System.Net.Sockets;

namespace NetworkInputMethod
{
    public class CavanTcpPacketClient : CavanTcpClient
    {
        private byte[] mBytes = new byte[32];

        public CavanTcpPacketClient(CavanTcpService service, TcpClient client) : base(service, client)
        {
        }

        public override void mainLoop()
        {
            NetworkStream stream = mClient.GetStream();
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
            mService.onTcpPacketReceived(this, bytes, length);
        }

        public override bool send(byte[] bytes, int offset, int length)
        {
            lock (this)
            {
                if (mClient == null)
                {
                    return false;
                }

                byte[] header = { (byte)(length & 0xFF), (byte)(length >> 8) };

                try
                {
                    NetworkStream stream = mClient.GetStream();
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