using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Windows.Forms;

namespace NetworkInputMethod
{
    public class CavanTcpServiceBase
    {
        private int mPort = 8865;
        private Thread mThread;
        private TcpListener mListener;
        private HashSet<CavanTcpClient> mClients = new HashSet<CavanTcpClient>();

        public bool Running
        {
            get
            {
                lock (this)
                {
                    return (mListener != null);
                }
            }
        }

        public TcpListener Listener
        {
            get
            {
                lock (this)
                {
                    return mListener;
                }
            }

            set
            {
                lock (this)
                {
                    mListener = value;
                }
            }
        }

        public virtual FormTcpService Form
        {
            get
            {
                return null;
            }

            set
            {
            }
        }

        public TcpListener setListener(TcpListener listener)
        {
            TcpListener backup;

            lock (this)
            {
                backup = mListener;
                mListener = listener;
            }

            return backup;
        }

        public IPEndPoint LocalEndpoint
        {
            get
            {
                var listener = Listener;
                if (listener == null)
                {
                    return null;
                }

                try
                {
                    return listener.Server.LocalEndPoint as IPEndPoint;
                }
                catch (Exception)
                {
                    return null;
                }
            }
        }

        public int Port
        {
            get
            {
                var addr = LocalEndpoint;
                if (addr == null)
                {
                    return mPort;
                }

                return addr.Port;
            }

            set
            {
                mPort = value;
            }
        }

        public string Url
        {
            get
            {
                return "127.0.0.1:" + Port;
            }
        }

        public HashSet<CavanTcpClient> Clients
        {
            get
            {
                return mClients;
            }
        }

        public int Count
        {
            get
            {
                lock (mClients)
                {
                    return mClients.Count;
                }
            }
        }

        public void start()
        {
            lock (this)
            {
                if (mThread == null)
                {
                    mThread = new Thread(new ThreadStart(runServiceThread))
                    {
                        IsBackground = true
                    };

                    mThread.Start();
                }
            }
        }

        public void stop()
        {
            var listener = setListener(null);
            if (listener != null)
            {
                listener.Stop();
            }
        }

        public void toggle(int port)
        {
            if (Running)
            {
                stop();
            }
            else
            {
                mPort = port;
                start();
            }
        }

        public void toggle(string port)
        {
            toggle(Convert.ToUInt16(port));
        }

        private void runServiceThread()
        {
            onTcpServiceStarted();

            TcpListener listener = new TcpListener(IPAddress.Any, mPort);

            try
            {
                listener.Start();
                mListener = listener;
                onTcpServiceRunning();

                while (true)
                {
                    TcpClient conn = listener.AcceptTcpClient();
                    if (conn == null)
                    {
                        break;
                    }

                    ParameterizedThreadStart start = new ParameterizedThreadStart(runTcpClientThread);
                    Thread thread = new Thread(start)
                    {
                        IsBackground = true
                    };

                    thread.Start(conn);
                }
            }
            catch (SocketException e)
            {
                if (e.SocketErrorCode != SocketError.Interrupted)
                {
                    MessageBox.Show(e.ToString());
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }
            finally
            {
                mListener = null;
                listener.Stop();
            }

            lock (this)
            {
                mThread = null;
            }

            onTcpServiceStopped();

            CavanTcpClient[] clients;

            lock (mClients)
            {
                clients = new CavanTcpClient[mClients.Count];
                mClients.CopyTo(clients, 0);
            }

            foreach (var client in clients)
            {
                client.disconnect();
            }
        }

        protected virtual void onTcpServiceRunning()
        {
        }

        protected virtual void onTcpServiceStarted()
        {
        }

        protected virtual void onTcpServiceStopped()
        {
        }

        protected virtual void onTcpServiceWaiting()
        {
        }

        protected virtual void onTcpClientConnected(CavanTcpClient client)
        {
        }

        protected virtual void onTcpClientDisconnected(CavanTcpClient client)
        {
        }

        public virtual void onTcpClientUpdated(CavanTcpClient client)
        {
        }

        public virtual CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            return null;
        }

        public virtual void onTcpCommandReceived(CavanTcpClient client, string[] args)
        {
        }

        public virtual void onTcpPacketReceived(CavanTcpClient client, byte[] bytes, int length)
        {
        }

        public virtual void onTcpDataReceived(CavanTcpClient client, byte[] bytes, int length)
        {
        }

        private void runTcpClientThread(object obj)
        {
            TcpClient conn = obj as TcpClient;
            CavanTcpClient client = onTcpClientAccepted(conn);

            if (client != null)
            {
                client.Service = this;

                client.onConnected();

                try
                {
                    if (client.mainLoop())
                    {
                        return;
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                }

                client.disconnect();
            }
            else
            {
                try
                {
                    conn.Close();
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                }
            }


        }

        internal void performTcpClientConnected(CavanTcpClient client)
        {
            bool added;

            lock (mClients)
            {
                added = mClients.Add(client);
            }

            if (added)
            {
                onTcpClientConnected(client);
            }
        }

        internal void performTcpClientDisconnected(CavanTcpClient client)
        {
            bool removed;

            lock (mClients)
            {
                removed = mClients.Remove(client);
            }

            if (removed)
            {
                onTcpClientDisconnected(client);
            }
        }
    }
}
