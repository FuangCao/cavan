using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace NetworkInputMethod
{
    public class CavanTcpService
    {
        private int mPort = 8865;
        private bool mEnabled;
        private TcpListener mListener;
        private Thread mThread;
        private List<CavanTcpClient> mClients = new List<CavanTcpClient>();

        public CavanTcpService()
        {
            mThread = new Thread(new ThreadStart(runServiceThread));
        }

        public int Port
        {
            get
            {
                return mPort;
            }

            set
            {
                if (mPort != value)
                {
                    mPort = value;

                    if (mListener != null)
                    {
                        mListener.Stop();
                    }
                }
            }
        }

        public void start()
        {
            mEnabled = true;

            Monitor.Enter(mThread);

            if (mThread.IsAlive)
            {

                Monitor.Pulse(mThread);

            }
            else
            {
                mThread.Start();
            }

            Monitor.Exit(mThread);
        }

        public void stop()
        {
            mEnabled = false;

            if (mListener != null)
            {
                mListener.Stop();
            }
        }

        private void runServiceThread()
        {
            while (true)
            {
                onTcpServiceStarted();

                while (mEnabled)
                {
                    try
                    {
                        mListener = new TcpListener(IPAddress.Any, mPort);
                        mListener.Start();
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine(e);
                        continue;
                    }

                    onTcpServiceRunning();

                    while (mEnabled)
                    {
                        try
                        {
                            TcpClient conn = mListener.AcceptTcpClient();
                            if (conn == null)
                            {
                                break;
                            }

                            if (mEnabled)
                            {
                                ParameterizedThreadStart start = new ParameterizedThreadStart(runTcpClientThread);
                                Thread thread = new Thread(start);
                                thread.Start(conn);
                            }
                        }
                        catch (Exception e)
                        {
                            Console.WriteLine(e);
                        }
                    }

                    try
                    {
                        mListener.Stop();
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine(e);
                    }

                    mListener = null;

                    lock (mClients)
                    {
                        foreach (CavanTcpClient client in mClients)
                        {
                            client.disconnect();
                        }
                    }

                    if (mEnabled)
                    {
                        Monitor.Enter(mThread);
                        Monitor.Wait(mThread, 2000);
                        Monitor.Exit(mThread);
                    }
                }

                onTcpServiceStopped();

                Monitor.Enter(mThread);
                Monitor.Wait(mThread);
                Monitor.Exit(mThread);
            }
        }

        protected virtual void onTcpServiceStopped()
        {
            Console.WriteLine("onTcpServiceStopped");
        }

        protected virtual void onTcpServiceStarted()
        {
            Console.WriteLine("onTcpServiceStarted");
        }

        protected virtual void onTcpServiceRunning()
        {
            Console.WriteLine("onTcpServiceRunning");
        }

        private void runTcpClientThread(object obj)
        {
            TcpClient conn = obj as TcpClient;
            CavanTcpClient client = onTcpClientAccepted(conn);

            if (client != null)
            {
                lock (mClients)
                {
                    mClients.Add(client);
                }

                onTcpClientConnected(client);

                try
                {
                    client.mainLoop();
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                }

                lock (mClients)
                {
                    mClients.Remove(client);
                }

                onTcpClientDisconnected(client);
            }

            try
            {
                conn.Close();
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }
        }

        protected virtual void onTcpClientDisconnected(CavanTcpClient client)
        {
            Console.WriteLine("onTcpClientDisconnected: " + client);
        }

        protected virtual void onTcpClientConnected(CavanTcpClient client)
        {
            Console.WriteLine("onTcpClientConnected: " + client);
        }

        protected virtual CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            return new CavanTcpClient(conn);
        }
    }
}
