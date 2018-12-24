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
                mIme.sendCommand(text, false);
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
    }
}
