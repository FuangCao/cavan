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
        private byte[] mRepeatBytes;
        private int mRepeatTimes;

        public FormSendCommand(FormNetworkIme ime)
        {
            mIme = ime;
            InitializeComponent();

            comboBoxDevices.Items.Add("所有的设备");
            comboBoxDevices.SelectedIndex = 0;

            comboBoxRepeat.SelectedIndex = 0;
        }

        private bool sendCommand(string command)
        {
            if (string.IsNullOrEmpty(command))
            {
                return false;
            }

            var item = comboBoxDevices.SelectedItem;

            if (item is NetworkImeClient)
            {
                var client = item as NetworkImeClient;
                return client.send(command);
            }
            else
            {
                mIme.sendCommand(command, false);
            }

            return true;
        }

        private void buttonSend_Click(object sender, EventArgs e)
        {
            sendCommand(textBoxCommand.Text.Trim());
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

            if (comboBoxDevices.SelectedIndex < 0)
            {
                comboBoxDevices.SelectedIndex = 0;
            }
        }

        private void buttonTest_Click(object sender, EventArgs e)
        {
            sendCommand("TEST");
        }

        private void buttonVip_Click(object sender, EventArgs e)
        {
            startRepeater("WEB vip");
        }

        private void buttonXfzd_Click(object sender, EventArgs e)
        {
            startRepeater("WEB xfzd");
        }

        private void buttonStop_Click(object sender, EventArgs e)
        {
            mRepeatBytes = null;
            timerRepeater.Enabled = false;
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            string command = textBoxCommand.Text;
            startRepeater(command.Trim());
        }

        private bool startRepeater(string command)
        {
            if (string.IsNullOrEmpty(command))
            {
                return false;
            }

            mRepeatBytes = UTF8Encoding.UTF8.GetBytes(command);
            mRepeatTimes = 0;

            if (updateRepeatInterval())
            {
                timerRepeater.Enabled = true;
            }

            return true;
        }

        private bool updateRepeatInterval()
        {
            int index = comboBoxRepeat.SelectedIndex;
            if (index < 20)
            {
                if (index > 0)
                {
                    timerRepeater.Interval = index * 50;
                }
                else
                {
                    timerRepeater.Enabled = false;
                    return false;
                }
            }
            else
            {
                timerRepeater.Interval = (index - 20) * 500 + 1000;
            }

            return true;
        }

        private void timerRepeater_Tick(object sender, EventArgs e)
        {
            byte[] bytes = mRepeatBytes;

            if (bytes == null)
            {
                timerRepeater.Enabled = false;
            }
            else
            {
                mIme.sendCommand(bytes, true);
                mRepeatTimes++;

                labelStatus.Text = "重复发送次数：" + mRepeatTimes;
            }
        }

        private void FormSendCommand_FormClosing(object sender, FormClosingEventArgs e)
        {
            buttonStop.PerformClick();
        }

        private void comboBoxRepeat_SelectedIndexChanged(object sender, EventArgs e)
        {
            updateRepeatInterval();
        }
    }
}
