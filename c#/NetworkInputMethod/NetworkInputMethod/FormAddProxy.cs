using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace NetworkInputMethod
{
    public partial class FormAddProxy : Form
    {
        public FormAddProxy()
        {
            InitializeComponent();
        }

        public int Port
        {
            get
            {
                try
                {
                    return Convert.ToUInt16(textBoxPort.Text);
                }
                catch (Exception)
                {
                    return 0;
                }
            }

            set
            {
                textBoxPort.Text = value.ToString();
            }
        }

        public string ServerUrl
        {
            get
            {
                return textBoxServer.Text;
            }

            set
            {
                textBoxServer.Text = value;
            }
        }

        public string ClientAddress
        {
            get
            {
                return textBoxClientAddress.Text;
            }

            set
            {
                textBoxClientAddress.Text = value;
            }
        }

        public string ClientName
        {
            get
            {
                return textBoxClientName.Text;
            }

            set
            {
                textBoxClientName.Text = value;
            }
        }

        public string TargetUrl
        {
            get
            {
                return textBoxTarget.Text;
            }

            set
            {
                textBoxTarget.Text = value;
            }
        }

        private void ButtonRefresh_Click(object sender, EventArgs e)
        {
            var url = textBoxServer.Text;

            if (url != null && url.Length > 0 && backgroundWorkerRefresh.IsBusy == false)
            {
                buttonRefresh.Enabled = false;
                backgroundWorkerRefresh.RunWorkerAsync(url);
            }
        }

        private void BackgroundWorkerRefresh_DoWork(object sender, DoWorkEventArgs e)
        {
            var url = e.Argument as string;
            e.Result = ReverseProxySlave.ListClients(url);
        }

        private void BackgroundWorkerRefresh_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            var clients = e.Result as string[];
            if (clients != null)
            {
                listBoxClients.Items.Clear();

                foreach (var client in clients)
                {
                    listBoxClients.Items.Add(client);
                }
            }

            buttonRefresh.Enabled = true;
        }

        private void ListBoxClients_DoubleClick(object sender, EventArgs e)
        {
            var item = listBoxClients.SelectedItem as string;
            if (item != null)
            {
                var args = item.Split(' ');

                if (args.Length > 3)
                {
                    textBoxClientName.Text = args[3].Trim();
                }
                else if (args.Length > 2)
                {
                    textBoxClientAddress.Text = args[2].Trim();
                }
            }
        }
    }
}
