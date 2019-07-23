using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Windows.Forms;

namespace NetworkInputMethod
{
    public abstract class FormTcpService : CavanSubForm
    {
        public abstract CavanTcpClient onTcpClientAccepted(TcpClient conn);

        public bool Invoke(EventHandler handler, object sender, EventArgs e)
        {
            try
            {
                base.Invoke(handler, sender, e);
                return true;
            }
            catch (Exception)
            {
                return false;
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
