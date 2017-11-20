using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Net.Sockets;

namespace NetworkInputMethod
{
    public partial class FormNetworkIme : Form
    {
        private NetworkImeService mService;

        delegate void SimpleDelegate(Object obj);

        public FormNetworkIme()
        {
            InitializeComponent();
            mService = new NetworkImeService(this);
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            mService.start();
        }

        private void buttonStop_Click(object sender, EventArgs e)
        {
            mService.stop();
        }

        public void onTcpClientConnected(object sender, EventArgs e)
        {
            Console.WriteLine("onTcpClientConnected: sender = " + sender);
            checkedListBoxClients.Items.Add(sender);
        }

        public void onTcpClientDisconnected(object sender, EventArgs e)
        {
            Console.WriteLine("onTcpClientDisconnected: sender = " + sender);
            checkedListBoxClients.Items.Remove(sender);
        }
    }

    public class NetworkImeClient : CavanTcpPacketClient
    {
        public NetworkImeClient(TcpClient client) : base(client)
        {
        }

        protected override void onDataPacketReceived(byte[] bytes, int length)
        {
            char[] chars = UTF8Encoding.ASCII.GetChars(bytes, 0, length);
            string command = new String(chars);
            Console.WriteLine("onDataPacketReceived: " + command);
        }
    }

    public class NetworkImeService : CavanTcpService
    {
        private FormNetworkIme mForm;

        public NetworkImeService(FormNetworkIme form)
        {
            mForm = form;
        }

        protected override CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            return new NetworkImeClient(conn);
        }

        protected override void onTcpClientConnected(CavanTcpClient client)
        {
            EventHandler handler = new EventHandler(mForm.onTcpClientConnected);
            mForm.Invoke(handler, client, null);
        }

        protected override void onTcpClientDisconnected(CavanTcpClient client)
        {
            EventHandler handler = new EventHandler(mForm.onTcpClientDisconnected);
            mForm.Invoke(handler, client, null);
        }
    }
}
