using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace NetworkInputMethod
{
    public class CavanThread
    {
        private string mName;
        private bool mRunning;
        private Thread mThread;
        private ThreadStart mMainLoop;
        private Queue<CavanHttpPacket> mPackets = new Queue<CavanHttpPacket>();

        public CavanThread(string name, ThreadStart loop)
        {
            mThread = new Thread(new ThreadStart(MainLoop))
            {
                IsBackground = true
            };

            mMainLoop = loop;
            mName = name;
        }

        private void MainLoop()
        {
            Monitor.Enter(this);

            while (true)
            {
                mRunning = true;
                Monitor.Exit(this);

                Console.WriteLine(mName + " thread running");

                try
                {
                    mMainLoop();
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                }

                Console.WriteLine(mName + " thread stopped");

                Monitor.Enter(this);
                mRunning = false;

                Monitor.Wait(this);
            }
        }

        public void Start()
        {
            lock (this)
            {
                Clear();

                if (mThread.IsAlive)
                {
                    Monitor.Pulse(this);
                }
                else
                {
                    mThread.Start();
                }
            }
        }

        public void Stop()
        {
            lock (this)
            {
                if (mRunning)
                {
                    Enqueue(null);
                }
            }
        }

        public string Name
        {
            get
            {
                return mName;
            }

            set
            {
                mName = value;
            }
        }

        public bool Running
        {
            get
            {
                lock (this)
                {
                    return mRunning;
                }
            }
        }

        public CavanHttpPacket Dequeue()
        {
            lock (mPackets)
            {
                while (true)
                {
                    if (mPackets.Count > 0)
                    {
                        return mPackets.Dequeue();
                    }

                    // Console.WriteLine(mName + " thread waitting");
                    Monitor.Wait(mPackets);
                    // Console.WriteLine(mName + " thread resume");
                }
            }
        }

        public void Enqueue(CavanHttpPacket packet)
        {
            lock (mPackets)
            {
                mPackets.Enqueue(packet);

                if (mPackets.Count == 1)
                {
                    Monitor.Pulse(mPackets);
                }
            }
        }

        public void Clear()
        {
            lock (mPackets)
            {
                mPackets.Clear();
            }
        }
    }
}
