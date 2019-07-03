using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Windows.Forms;

namespace NetworkInputMethod
{
    public class CavanTcpService : CavanTcpServiceBase
    {
        private FormTcpService mForm;

        public CavanTcpService(FormTcpService form)
        {
            mForm = form;
        }

        public override FormTcpService Form
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

        protected override void onTcpServiceRunning()
        {
            EventHandler handler = new EventHandler(mForm.onTcpServiceRunning);
            Invoke(handler, this, null);
        }

        protected override void onTcpServiceStarted()
        {
            EventHandler handler = new EventHandler(mForm.onTcpServiceStarted);
            Invoke(handler, this, null);
        }

        protected override void onTcpServiceStopped()
        {
            EventHandler handler = new EventHandler(mForm.onTcpServiceStopped);
            Invoke(handler, this, null);
        }

        protected override void onTcpServiceWaiting()
        {
            EventHandler handler = new EventHandler(mForm.onTcpServiceWaiting);
            Invoke(handler, this, null);
        }

        protected override void onTcpClientConnected(CavanTcpClient client)
        {
            EventHandler handler = new EventHandler(mForm.onTcpClientConnected);
            Invoke(handler, client, null);
        }

        protected override void onTcpClientDisconnected(CavanTcpClient client)
        {
            EventHandler handler = new EventHandler(mForm.onTcpClientDisconnected);
            Invoke(handler, client, null);
        }

        public override void onTcpClientUpdated(CavanTcpClient client)
        {
            EventHandler handler = new EventHandler(mForm.onTcpClientUpdated);
            Invoke(handler, client, null);
        }

        public override CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            return mForm.onTcpClientAccepted(conn);
        }

        public override void onTcpCommandReceived(CavanTcpClient client, string[] args)
        {
            mForm.onTcpCommandReceived(client, args);
        }

        public override void onTcpPacketReceived(CavanTcpClient client, byte[] bytes, int length)
        {
            mForm.onTcpPacketReceived(client, bytes, length);
        }

        public override void onTcpDataReceived(CavanTcpClient client, byte[] bytes, int length)
        {
            mForm.onTcpDataReceived(client, bytes, length);
        }
    }
}
