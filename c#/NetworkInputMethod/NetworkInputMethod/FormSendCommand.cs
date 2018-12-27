using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace NetworkInputMethod
{
    public partial class FormSendCommand : Form
    {
        private FormNetworkIme mIme;

        public FormSendCommand(FormNetworkIme ime)
        {
            mIme = ime;
            InitializeComponent();
        }

        private void buttonSend_Click(object sender, EventArgs e)
        {
            var text = textBoxCommand.Text.Trim();
            if (text.Length > 0)
            {
                var item = comboBoxDevices.SelectedItem;

                if (item is NetworkImeClient)
                {
                    var client = item as NetworkImeClient;
                    client.send(text);
                }
                else
                {
                    mIme.sendCommand(text, false);
                }
            }
        }

        private void buttonView_Click(object sender, EventArgs e)
        {
            var url = textBoxCommand.Text.Trim();
            if (url.Length > 0)
            {
                mIme.sendCommand("VIEW " + url, false);
            }
        }

        internal void postResponse(string response)
        {
            using (var reader = new StringReader(response))
            {
                while (true)
                {
                    var line = reader.ReadLine();
                    if (line == null)
                    {
                        break;
                    }

                    textBoxResponse.AppendText(line);
                    textBoxResponse.AppendText("\r\n");
                }
            }
        }

        private void buttonClear_Click(object sender, EventArgs e)
        {
            textBoxResponse.Clear();
        }

        private void comboBoxDevices_DropDown(object sender, EventArgs e)
        {
            var items = comboBoxDevices.Items;

            items.Clear();
            items.Add("所有的设备");

            foreach (var item in mIme.NetworkImeClients)
            {
                items.Add(item);
            }
        }

        private void buttonTest_Click(object sender, EventArgs e)
        {
            mIme.sendCommand("TEST", false);
        }
    }
}
