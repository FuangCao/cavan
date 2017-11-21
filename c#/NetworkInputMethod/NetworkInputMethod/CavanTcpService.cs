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
                        mListener = null;
                    }
                }
            }
        }

        public void start()
        {
            lock (this)
            {
                mEnabled = true;

                lock (mThread)
                {
                    if (mThread.IsAlive)
                    {

                        Monitor.Pulse(mThread);

                    }
                    else
                    {
                        mThread.Start();
                    }
                }
            }
        }

        public void stop()
        {
            lock (this)
            {
                mEnabled = false;

                if (mListener != null)
                {
                    mListener.Stop();
                    mListener = null;
                }
            }
        }

        private void runServiceThread()
        {
            while (true)
            {
                onTcpServiceStarted();

                while (mEnabled)
                {
                    TcpListener listener = null;

                    try
                    {
                        listener = new TcpListener(IPAddress.Any, mPort);
                        listener.Start();
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine(e);

                        if (listener != null)
                        {
                            listener.Stop();
                        }

                        continue;
                    }

                    mListener = listener;
                    onTcpServiceRunning();

                    while (mEnabled)
                    {
                        try
                        {
                            TcpClient conn = listener.AcceptTcpClient();
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
                        listener.Stop();
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
                        lock (mThread)
                        {
                            Monitor.Wait(mThread, 2000);
                        }
                    }
                }

                onTcpServiceStopped();

                lock (mThread)
                {
                    Monitor.Wait(mThread);
                }
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
