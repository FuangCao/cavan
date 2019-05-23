using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Windows.Forms;

namespace NetworkInputMethod
{
    public class FormTcpService : Form
    {
        public void Invoke(EventHandler handler, object sender, EventArgs e)
        {
            if (!IsDisposed)
            {
                base.Invoke(handler, sender, e);
            }
        }

        public virtual void onTcpServiceStopped(object sender, EventArgs e)
        {
            Console.WriteLine("onTcpServiceStopped");
        }

        public virtual void onTcpServiceStarted(object sender, EventArgs e)
        {
            Console.WriteLine("onTcpServiceStarted");
        }

        public virtual void onTcpServiceRunning(object sender, EventArgs e)
        {
            Console.WriteLine("onTcpServiceRunning");
        }

        public virtual void onTcpServiceWaiting(object sender, EventArgs e)
        {
            Console.WriteLine("onTcpServiceWaiting");
        }

        public virtual void onTcpClientConnected(object sender, EventArgs e)
        {
            Console.WriteLine("onTcpClientConnected");
        }

        public virtual void onTcpClientDisconnected(object sender, EventArgs e)
        {
            Console.WriteLine("onTcpClientDisconnected");
        }

        public virtual void onTcpClientUpdated(object sender, EventArgs e)
        {
            Console.WriteLine("onTcpClientUpdated");
        }

        public virtual CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            throw new NotImplementedException();
        }

        public virtual void onTcpCommandReceived(CavanTcpClient client, string[] args)
        {
            Invoke(new EventHandler(onTcpCommandReceived), client, new CavanEventArgs<string[]>(args));
        }

        public virtual void onTcpCommandReceived(object sender, EventArgs e)
        {
            throw new NotImplementedException();
        }

        public virtual void onTcpPacketReceived(CavanTcpClient client, byte[] bytes, int length)
        {
            throw new NotImplementedException();
        }

        public virtual void onTcpDataReceived(CavanTcpClient client, byte[] bytes, int length)
        {
            throw new NotImplementedException();
        }
    }
}
