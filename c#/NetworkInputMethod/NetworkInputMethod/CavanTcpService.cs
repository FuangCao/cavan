using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Windows.Forms;

namespace NetworkInputMethod
{
    public class CavanTcpService
    {
        private int mPort = 8865;
        private TcpListener mListener;
        private Thread mThread;
        private FormTcpService mForm;
        private List<CavanTcpClient> mClients = new List<CavanTcpClient>();

        public CavanTcpService(FormTcpService form)
        {
            mForm = form;
        }

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

        public FormTcpService Form
        {
            get
            {
                return mForm;
            }
        }

        public void Invoke(EventHandler handler, object sender, EventArgs e)
        {
            mForm.Invoke(handler, sender, e);
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
            lock (this)
            {
                if (mListener != null)
                {
                    mListener.Stop();
                    mListener = null;
                }
            }
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

            lock (mClients)
            {
                foreach (CavanTcpClient client in mClients)
                {
                    client.disconnect();
                }
            }
        }

        protected void onTcpServiceRunning()
        {
            EventHandler handler = new EventHandler(mForm.onTcpServiceRunning);
            Invoke(handler, this, null);
        }

        protected virtual void onTcpServiceStarted()
        {
            EventHandler handler = new EventHandler(mForm.onTcpServiceStarted);
            Invoke(handler, this, null);
        }

        protected virtual void onTcpServiceStopped()
        {
            EventHandler handler = new EventHandler(mForm.onTcpServiceStopped);
            Invoke(handler, this, null);
        }

        protected virtual void onTcpServiceWaiting()
        {
            EventHandler handler = new EventHandler(mForm.onTcpServiceWaiting);
            Invoke(handler, this, null);
        }

        protected virtual void onTcpClientConnected(CavanTcpClient client)
        {
            EventHandler handler = new EventHandler(mForm.onTcpClientConnected);
            Invoke(handler, client, null);
        }

        protected virtual void onTcpClientDisconnected(CavanTcpClient client)
        {
            EventHandler handler = new EventHandler(mForm.onTcpClientDisconnected);
            Invoke(handler, client, null);
        }

        public virtual void onTcpClientUpdated(CavanTcpClient client)
        {
            EventHandler handler = new EventHandler(mForm.onTcpClientUpdated);
            Invoke(handler, client, null);
        }

        public virtual CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            return mForm.onTcpClientAccepted(conn);
        }

        public virtual void onTcpCommandReceived(CavanTcpClient client, string[] args)
        {
            mForm.onTcpCommandReceived(client, args);
        }

        public virtual void onTcpPacketReceived(CavanTcpClient client, byte[] bytes, int length)
        {
            mForm.onTcpPacketReceived(client, bytes, length);
        }

        public virtual void onTcpDataReceived(CavanTcpClient client, byte[] bytes, int length)
        {
            mForm.onTcpDataReceived(client, bytes, length);
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
    }
}
