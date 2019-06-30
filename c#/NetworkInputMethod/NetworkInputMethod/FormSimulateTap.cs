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
        private int mAxisX;
        private int mAxisY;
        private FormNetworkIme mIme;

        public FormSimulateTap(FormNetworkIme ime)
        {
            mIme = ime;
            InitializeComponent();
            comboBoxStep.Text = "100";
        }

        public int AxisX
        {
            get
            {
                return mAxisX;
            }

            set
            {
                if (value < 0)
                {
                    mAxisX = 0;
                }
                else
                {
                    mAxisX = value;
                }

                textBoxAxisX.Text = mAxisX.ToString();
            }
        }

        public int AxisY
        {
            get
            {
                return mAxisY;
            }

            set
            {
                if (value < 0)
                {
                    mAxisY = 0;
                }
                else
                {
                    mAxisY = value;
                }

                textBoxAxisY.Text = mAxisY.ToString();
            }
        }

        public bool apply()
        {
            return sendCommand("cursor set " + mAxisX + " " + mAxisY);
        }

        public bool setAxisX(int value)
        {
            AxisX = value;
            return apply();
        }

        public bool setAxisY(int value)
        {
            AxisY = value;
            return apply();
        }

        public bool setAxis(int x, int y)
        {
            AxisX = x;
            AxisY = y;
            return apply();
        }

        public bool sendCommand(string command)
        {
            var client = comboBoxDevices.SelectedItem as NetworkImeClient;
            if (client == null)
            {
                return false;
            }

            return client.send(command);
        }

        private void comboBoxStep_TextChanged(object sender, EventArgs e)
        {
            mStep = Convert.ToInt32(comboBoxStep.Text);
        }

        private void comboBoxDevices_DropDown(object sender, EventArgs e)
        {
            var items = comboBoxDevices.Items;

            items.Clear();

            foreach (var item in mIme.NetworkImeClients)
            {
                items.Add(item);
            }

            if (comboBoxDevices.SelectedIndex < 0 && items.Count > 0)
            {
                comboBoxDevices.SelectedIndex = 0;
            }

        }

        private void buttonZero_Click(object sender, EventArgs e)
        {
            setAxis(0, 0);
            comboBoxStep.Text = "100";
        }

        private void buttonSave_Click(object sender, EventArgs e)
        {
            apply();
            sendCommand("cursor save");
        }

        private void buttonRemove_Click(object sender, EventArgs e)
        {
            sendCommand("cursor remove");
        }

        private void buttonTap_Click(object sender, EventArgs e)
        {
            sendCommand("signin");
        }

        private void buttonOpen_Click(object sender, EventArgs e)
        {
            sendCommand("cursor 1");
        }

        private void buttonClose_Click(object sender, EventArgs e)
        {
            sendCommand("cursor 0");
        }

        private void buttonUp_Click(object sender, EventArgs e)
        {
            setAxisY(mAxisY - mStep);
        }

        private void buttonDown_Click(object sender, EventArgs e)
        {
            setAxisY(mAxisY + mStep);
        }

        private void buttonLeft_Click(object sender, EventArgs e)
        {
            setAxisX(mAxisX + mStep);
        }

        private void buttonRight_Click(object sender, EventArgs e)
        {
            setAxisX(mAxisX - mStep);
        }
    }
}
