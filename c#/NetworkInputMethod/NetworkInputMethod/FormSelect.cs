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
    public partial class FormSelect : Form
    {
        private FormNetworkIme mNetworkIme;

        public FormSelect(FormNetworkIme ime)
        {
            mNetworkIme = ime;
            InitializeComponent();
            Console.WriteLine("AcceptButton = " + AcceptButton);
        }

        private void buttonUp_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendKey(19);
        }

        private void buttonDown_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendKey(20);
        }

        private void buttonLeft_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendKey(21);
        }

        private void buttonRight_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendKey(22);
        }

        private void buttonEnter_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendKey(66);
        }

        private void buttonSelectAll_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendCommand("SELECT_ALL", false);
        }

        private void buttonBack_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendCommand("BACK", false);
        }

        private void buttonHome_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendCommand("HOME", false);
        }
    }
}
