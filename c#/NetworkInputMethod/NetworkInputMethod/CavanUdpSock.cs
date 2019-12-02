using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace NetworkInputMethod
{
    public class CavanUdpSock
    {
        public const int MAX_LINKS = 1 << 16;
        public const int WIN_SIZE = 16;

        public const int CMD_PING = 0;
        public const int CMD_OPEN = 1;
        public const int CMD_CLOSE = 2;
        public const int CMD_DATA = 3;
        public const int CMD_ACK = 5;
        public const int CMD_ERR = 4;

        private CavanUdpLink[] mLinks = new CavanUdpLink[MAX_LINKS];
        private CavanUdpPack mSendPacks;
        private Thread mSendThread;
        private Thread mRecvThread;
        private Socket mSocket;
        private int mIndex;
        private int mPort;

        public CavanUdpSock(int port)
        {
            mLinks[0] = new CavanUdpService();
            mPort = port;
        }

        public Socket Open(int port)
        {
            var sock = mSocket;
            if (sock != null)
            {
                return sock;
            }

            try
            {
                sock = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);

                if (port > 0)
                {
                    sock.Bind(new IPEndPoint(IPAddress.Any, port));
                }

                mSocket = sock;

                if (mSendThread == null)
                {
                    mSendThread = new Thread(new ThreadStart(RunSendThread))
                    {
                        IsBackground = true
                    };

                    mSendThread.Start();
                }
                else
                {
                    lock (mSendThread)
                    {
                        Monitor.Pulse(mSendThread);
                    }
                }

                if (mRecvThread == null)
                {
                    mRecvThread = new Thread(new ThreadStart(RunRecvThread))
                    {
                        IsBackground = true
                    };

                    mRecvThread.Start();
                }
                else
                {
                    lock (mRecvThread)
                    {
                        Monitor.Pulse(mRecvThread);
                    }
                }

                return sock;
            }
            catch (Exception)
            {
                if (sock != null)
                {
                    sock.Close();
                }

                return null;
            }
        }

        public void Close()
        {
            var sock = mSocket;

            mSocket = null;

            if (sock != null)
            {
                sock.Close();
            }
        }

        public int AllocPort()
        {
            for (var index = mIndex; mIndex < MAX_LINKS; index++)
            {
                var node = mLinks[index];

                if (node == null)
                {
                    return index;
                }
            }

            for (var index = 1; index < mIndex; index++)
            {
                if (mLinks[index] == null)
                {
                    return index;
                }
            }

            var link = mLinks[1];

            for (var index = 2; index < MAX_LINKS; index++)
            {
                if (mLinks[index].LiveTime < link.LiveTime)
                {
                    link = mLinks[index];
                }
            }

            return link.LocalPort;
        }

        public void Bind(CavanUdpLink link)
        {
            lock (this)
            {
                var port = AllocPort();
                link.LocalPort = port;
                mLinks[port] = link;
            }
        }

        private void RunRecvThread()
        {
            while (true)
            {
                try
                {
                    var sock = mSocket;
                    if (sock == null)
                    {
                        continue;
                    }

                    var ep = new IPEndPoint(IPAddress.Any, 0) as EndPoint;
                    var bytes = new byte[4096];

                    while (true)
                    {
                        var length = sock.ReceiveFrom(bytes, ref ep);
                        if (length < 6)
                        {
                            if (length > 0)
                            {
                                continue;
                            }
                            else
                            {
                                break;
                            }
                        }

                        var data = new byte[length];
                        Array.Copy(bytes, 0, data, 0, length);
                        OnPackReceived(new CavanUdpPack(data));
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                }
                finally
                {
                    lock (mRecvThread)
                    {
                        Monitor.Wait(mRecvThread);
                    }
                }
            }
        }

        protected virtual void OnPackReceived(CavanUdpPack pack)
        {
            var link = mLinks[pack.DestPort];
            var command = pack.Command;

            if (link == null)
            {
                if (command == CMD_OPEN)
                {
                    OnOpenLink(pack);
                }
            }
            else if (command == CMD_ACK)
            {
                OnResponseReceived(pack);
            }
            else
            {
                OnCommandReceived(pack);
            }
        }

        protected virtual void OnCommandReceived(CavanUdpPack pack)
        {
            Console.WriteLine("OnCommandReceived");
        }

        protected virtual void OnResponseReceived(CavanUdpPack pack)
        {
            Console.WriteLine("OnResponseReceived");
        }

        protected virtual void OnOpenLink(CavanUdpPack pack)
        {
            Console.WriteLine("OnOpenLink");
        }

        public int SendTo(Socket sock, byte[] bytes, EndPoint ep)
        {
            lock (this)
            {
                return sock.SendTo(bytes, ep);
            }
        }

        public void Send(CavanUdpPack pack)
        {
            mSendPacks.Enqueue(pack);
        }

        private void RunSendThread()
        {
            while (true)
            {
                try
                {
                    var sock = mSocket;

                    while (true)
                    {
                        var pack = mSendPacks.Dequeue();
                        SendTo(sock, pack.Bytes, pack.Link.EndPoint);
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                }
                finally
                {
                    mSendPacks.Clear();

                    lock (mSendThread)
                    {
                        Monitor.Wait(mSendThread);
                    }
                }
            }
        }

        public class CavanUdpLink
        {
            private CavanUdpWindow mSendWin;
            private CavanUdpWindow mRecvWin;
            private IPEndPoint mEndPoint;
            private int mRemotePort;
            private int mLocalPort;
            private int mDelay;

            public CavanUdpLink()
            {
                mSendWin = new CavanUdpWindow(this);
                mRecvWin = new CavanUdpWindow(this);
            }

            public int RemotePort { get => mRemotePort; set => mRemotePort = value; }

            public int LocalPort { get => mLocalPort; set => mLocalPort = value; }

            public EndPoint EndPoint { get => mEndPoint; set => mEndPoint = value as IPEndPoint; }

            public CavanUdpSock Sock { get; set; }

            public int NetDelay { get; internal set; }

            public DateTime LiveTime { get; internal set; }

            public void OnPackReceived(int cmd, byte[] bytes, int length)
            {
                if (cmd == CMD_ACK)
                {
                }
                else
                {
                }
            }

            public void Send(int type, byte[] bytes)
            {
                var pack = new CavanUdpPack(bytes.Length)
                {
                    DestPort = mRemotePort,
                    SrcPort = mLocalPort,
                    Command = type,
                    Bytes = bytes,
                    Link = this
                };

                mSendWin.Write(pack);
            }

            public void Send(CavanUdpPack pack)
            {
                mSendWin.Write(pack);
                Sock.Send(pack);
            }

            public CavanUdpPack Recv()
            {
                return mRecvWin.Read();
            }

            public bool Connect(EndPoint ep, string url)
            {
                var pack = new CavanUdpPack(url.Length)
                {
                    Command = CMD_OPEN,
                    Link = this
                };

                return pack.SendSync();
            }
        }

        public class CavanUdpService : CavanUdpLink
        {

        }

        public class CavanUdpPack
        {
            public const int STATE_PENDING = 0;
            public const int STATE_SUCCESS = 1;
            public const int STATE_TIMEOUT = 2;
            public const int STATE_ERROR = 3;

            protected CavanUdpLink mLink;
            private DateTime mTime;
            protected byte[] mBytes;
            protected int mLength;
            protected int mTimes;
            protected int mState;

            public CavanUdpPack(byte[] bytes)
            {
                Next = Prev = this;
                mBytes = bytes;
                mLength = 6;
                mTimes = 5;
            }

            public CavanUdpPack(int length) : this(new byte[length + 6]) { }

            public CavanUdpLink Link
            {
                get
                {
                    return mLink;
                }

                set
                {
                    DestPort = value.RemotePort;
                    SrcPort = value.LocalPort;
                    mLink = value;
                }
            }

            public CavanUdpPack Next { get; set; }

            public CavanUdpPack Prev { get; set; }

            public int RetryTimes { get; set; }

            public int Times { get => mTimes; set => mTimes = value; }

            public int State
            {
                get
                {
                    lock (this)
                    {
                        return mState;
                    }
                }

                set
                {
                    lock (this)
                    {
                        mState = value;
                        Monitor.Pulse(this);
                    }
                }
            }

            public bool IsPending
            {
                get
                {
                    return (State == STATE_PENDING);
                }
            }


            public bool IsSuccess
            {
                get
                {
                    return (State == STATE_SUCCESS);
                }
            }

            public bool IsTimeout
            {
                get
                {
                    return (State == STATE_PENDING);
                }
            }

            public bool IsError
            {
                get
                {
                    return (State == STATE_ERROR);
                }
            }

            public void Append(CavanUdpPack pack)
            {
                pack.Next = Next;
                pack.Prev = this;
                Next.Prev = pack;
                Next = pack;
            }

            public void Insert(CavanUdpPack pack)
            {
                pack.Prev = Prev;
                pack.Next = this;
                Prev.Next = pack;
                Prev = pack;
            }

            public void Unlink()
            {
                Prev.Next = Next;
                Next.Prev = Prev;
                Next = Prev = this;
            }

            public void Enqueue(CavanUdpPack pack)
            {
                lock (this)
                {
                    var node = Next;

                    while (node != this && node.Time <= pack.Time)
                    {
                        node = node.Next;
                    }

                    node.Insert(pack);

                    if (node == this)
                    {
                        Monitor.Pulse(this);
                    }
                }
            }

            public CavanUdpPack Dequeue()
            {
                CavanUdpPack pack;
                DateTime time;

                lock (this)
                {
                    while (true)
                    {
                        pack = Next;

                        if (pack == null)
                        {
                            Monitor.Wait(this);
                            continue;
                        }

                        time = DateTime.Now;

                        if (pack.Time > time)
                        {
                            var delay = pack.Time - time;
                            Monitor.Wait(this, (int)delay.TotalMilliseconds);
                            continue;
                        }

                        break;
                    }

                    pack.Unlink();
                }

                if (pack.Times < pack.RetryTimes)
                {
                    pack.Time = time.AddMilliseconds(mLink.NetDelay);
                    Enqueue(pack);
                }
                else if (pack.RetryTimes > 0)
                {
                    pack.OnSendTimeout();
                }
                else
                {
                    pack.OnSendSuccess();
                }

                return pack;
            }

            protected virtual void OnSendSuccess()
            {
                Console.WriteLine("OnSendSuccess");
                State = STATE_SUCCESS;
            }

            protected virtual void OnSendTimeout()
            {
                Console.WriteLine("OnSendTimeout");
                State = STATE_TIMEOUT;
            }

            protected virtual void OnSendError()
            {
                Console.WriteLine("OnSendTimeout");
                State = STATE_ERROR;
            }

            public int DestPort
            {
                get
                {
                    return Utils.DecodeValue16(mBytes, 0);
                }

                set
                {
                    Utils.EncodeValue16(mBytes, 0, value);
                }
            }

            public int SrcPort
            {
                get
                {
                    return Utils.DecodeValue16(mBytes, 2);
                }

                set
                {
                    Utils.EncodeValue16(mBytes, 2, value);
                }
            }

            public int Index
            {
                get
                {
                    return Utils.DecodeValue16(mBytes, 4);
                }

                set
                {
                    Utils.EncodeValue16(mBytes, 4, value);
                }
            }

            public int Command
            {
                get
                {
                    return Utils.DecodeValue8(mBytes, 5);
                }

                set
                {
                    Utils.EncodeValue8(mBytes, 5, value);
                }
            }

            public byte[] Bytes
            {
                get
                {
                    return mBytes;
                }

                set
                {
                    Array.Copy(value, 0, mBytes, 7, value.Length);
                }
            }

            protected DateTime Time { get => mTime; set => mTime = value; }

            public void AppendValue8(int value)
            {
                if (mLength < mBytes.Length)
                {
                    mBytes[mLength] = (byte)value;
                    mLength++;
                }
            }

            public void AppendBool(bool value)
            {
                AppendValue8(value ? 0x01 : 0x00);
            }

            public void AppendValue16(int value)
            {
                AppendValue8(value);
                AppendValue8(value >> 8);
            }

            public void AppendValue24(int value)
            {
                AppendValue16(value);
                AppendValue8(value >> 16);
            }

            public void AppendValue32(int value)
            {
                AppendValue24(value);
                AppendValue8(value >> 24);
            }

            public int ReadValue8()
            {
                if (mLength < mBytes.Length)
                {
                    return mBytes[mLength++];
                }

                return 0;
            }

            public bool ReadBool()
            {
                return ReadValue8() != 0;
            }

            public int ReadValue16()
            {
                return ReadValue8() | ReadValue8() << 8;
            }

            public int ReadValue24()
            {
                return ReadValue16() | ReadValue8() << 16;
            }

            public int ReadValue32()
            {
                return ReadValue24() | ReadValue8() << 24;
            }

            public void Clear()
            {
                lock (this)
                {
                    for (var pack = Next; pack != this; pack = pack.Next)
                    {
                        pack.OnSendTimeout();
                    }

                    Next = Prev = this;
                }
            }

            public void SendAsync()
            {
                lock (this)
                {
                    mState = STATE_PENDING;
                    mTimes = 0;
                    mLink.Send(this);
                }
            }

            public bool SendSync()
            {
                SendAsync();

                lock (this)
                {
                    while (mState == STATE_PENDING)
                    {
                        Monitor.Wait(this);
                    }

                    return (mState == STATE_SUCCESS);
                }
            }
        }

        public class CavanUdpWindow
        {
            private CavanUdpPack[] mPacks = new CavanUdpPack[WIN_SIZE];
            private CavanUdpLink mLink;
            private ushort mIndex;

            public CavanUdpWindow(CavanUdpLink link)
            {
                mLink = link;
            }

            public void Write(CavanUdpPack pack)
            {
                Monitor.Enter(this);

                var index = mIndex & (WIN_SIZE - 1);

                while (mPacks[index] != null)
                {
                    Monitor.Wait(this);
                }

                mPacks[index] = pack;
                pack.Index = mIndex;

                Monitor.Exit(this);
            }

            public CavanUdpPack Read()
            {
                CavanUdpPack pack;

                Monitor.Enter(this);

                var index = mIndex & (WIN_SIZE - 1);

                while (true)
                {
                    pack = mPacks[index];
                    if (mPacks != null)
                    {
                        break;
                    }

                    Monitor.Wait(this);
                }

                Monitor.Exit(this);

                return pack;
            }

            public bool Enqueue(CavanUdpPack pack)
            {
                var index = pack.Index & (WIN_SIZE - 1);

                lock (this)
                {
                    if (mPacks[index] != null)
                    {
                        return false;
                    }

                    mPacks[index] = pack;

                    if (pack.Index == mIndex)
                    {
                        Monitor.Pulse(this);
                    }

                    return true;
                }
            }

            public bool Dequeue(CavanUdpPack pack)
            {
                var index = pack.Index & (WIN_SIZE - 1);

                lock (this)
                {
                    var node = mPacks[index];
                    if (node == null)
                    {
                        return false;
                    }

                    mPacks[index] = null;

                    if (index == (mIndex & (WIN_SIZE - 1)))
                    {
                        Monitor.Pulse(this);
                    }
                }

                return true;
            }
        }
    }
}
