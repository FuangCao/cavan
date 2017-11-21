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

        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            switch (keyData)
            {
                case Keys.Control | Keys.Enter:
                    buttonSend.PerformClick();
                    return true;
            }

            return base.ProcessCmdKey(ref msg, keyData);
        }

        public int sendCommand(byte[] bytes)
        {
            int count = 0;

            foreach (Object item in checkedListBoxClients.CheckedItems)
            {
                NetworkImeClient client = item as NetworkImeClient;
                if (!client.send(bytes))
                {
                    count++;
                }
            }

            return count;
        }

        public int sendCommand(string command)
        {
            byte[] bytes = UTF8Encoding.UTF8.GetBytes(command);
            return sendCommand(bytes);
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

        public void onTcpClientUpdated(object sender, EventArgs e)
        {
            Console.WriteLine("onTcpClientUpdated: sender = " + sender);
            checkedListBoxClients.Invalidate();
        }

        private void buttonSend_Click(object sender, EventArgs e)
        {
            string text = textBoxContent.Text;
            string command;

            if (radioButtonInsert.Checked)
            {
                command = "INSERT";
            }
            else if (radioButtonReplace.Checked)
            {
                command = "REPLACE";
            }
            else
            {
                command = "SEND";
            }

            if (text != null && text.Length > 0)
            {
                command += " " + text;
            }

            sendCommand(command);

            if (checkBoxClear.Checked)
            {
                textBoxContent.Clear();
            }
        }

        private void buttonClear_Click(object sender, EventArgs e)
        {
            textBoxContent.Clear();
            sendCommand("REPLACE");
        }
    }

    public class NetworkImeClient : CavanTcpPacketClient
    {
        private string mUserName;
        private NetworkImeService mService;

        public NetworkImeClient(NetworkImeService service, TcpClient client) : base(client)
        {
            mService = service;
        }

        protected override void onDataPacketReceived(byte[] bytes, int length)
        {
            char[] chars = UTF8Encoding.ASCII.GetChars(bytes, 0, length);
            string command = new String(chars);
            Console.WriteLine("onDataPacketReceived: " + command);
            string[] args = command.Split(new char[] { ' ' }, 2);
            Console.WriteLine("command = " + args[0]);

            switch (args[0])
            {
                case "USER":
                    if (args.Length > 1)
                    {
                        string name = args[1].Trim();
                        if (name.Length > 0)
                        {
                            mUserName = name;
                            FormNetworkIme form = mService.Form;
                            EventHandler handler = new EventHandler(form.onTcpClientUpdated);
                            form.Invoke(handler, this, null);
                        }

                        Console.WriteLine("user = " + mUserName);
                    }
                    break;
            }
        }

        public override string ToString()
        {
            string text = base.ToString();

            if (mUserName != null)
            {
                return text + " - " + mUserName;
            }

            return text;
        }
    }

    public class NetworkImeService : CavanTcpService
    {
        private FormNetworkIme mForm;

        public NetworkImeService(FormNetworkIme form)
        {
            mForm = form;
        }

        public FormNetworkIme Form
        {
            get
            {
                return mForm;
            }
        }

        protected override CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            return new NetworkImeClient(this, conn);
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
