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
    public partial class FormSimulateTap : Form
    {
        private int mStep;
        private FormNetworkIme mIme;

        public FormSimulateTap(FormNetworkIme ime)
        {
            mIme = ime;
            InitializeComponent();
            comboBoxDevices.SelectedIndex = 0;
            comboBoxType.SelectedIndex = 0;
            comboBoxStep.Text = "100";
        }

        public bool sendCommand(string command)
        {
            var bytes = Encoding.UTF8.GetBytes(command);

            if (comboBoxDevices.SelectedIndex > 0)
            {
                var client = comboBoxDevices.SelectedItem as NetworkImeClient;
                if (client == null)
                {
                    return false;
                }

                return client.send(bytes);
            }

            return mIme.sendCommand(bytes, false) > 0;
        }

        private void comboBoxStep_TextChanged(object sender, EventArgs e)
        {
            mStep = Convert.ToInt32(comboBoxStep.Text);
        }

        private void comboBoxDevices_DropDown(object sender, EventArgs e)
        {
            var items = comboBoxDevices.Items;
            var first = items[0];

            items.Clear();
            items.Add(first);

            foreach (var item in mIme.Clients)
            {
                items.Add(item);
            }

            if (comboBoxDevices.SelectedIndex < 0 && items.Count > 0)
            {
                comboBoxDevices.SelectedIndex = 0;
            }

        }

        private void buttonSave_Click(object sender, EventArgs e)
        {
            sendCommand("cursor save");
        }

        private void buttonRemove_Click(object sender, EventArgs e)
        {
            sendCommand("cursor remove");
        }

        private void buttonTap_Click(object sender, EventArgs e)
        {
            sendCommand("cursor tap");
        }

        private void buttonOpen_Click(object sender, EventArgs e)
        {
            sendCommand("cursor 1 " + comboBoxType.SelectedIndex);
        }

        private void buttonClose_Click(object sender, EventArgs e)
        {
            sendCommand("cursor 0");
        }

        public void sendCommandAdd(int x, int y)
        {
            sendCommand("cursor add " + x + " " + y);
        }

        private void buttonUp_Click(object sender, EventArgs e)
        {
            sendCommandAdd(0, -mStep);
        }

        private void buttonDown_Click(object sender, EventArgs e)
        {
            sendCommandAdd(0, mStep);
        }

        private void buttonLeft_Click(object sender, EventArgs e)
        {
            sendCommandAdd(-mStep, 0);
        }

        private void buttonRight_Click(object sender, EventArgs e)
        {
            sendCommandAdd(mStep, 0);
        }

        private void buttonLogin_Click(object sender, EventArgs e)
        {
            sendCommand("signin");
        }

        private void buttonLogout_Click(object sender, EventArgs e)
        {
            sendCommand("unfollow");
        }
    }
}
