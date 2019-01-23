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
    public partial class FormAlipay : Form
    {
        private FormNetworkIme mIme;

        public FormAlipay(FormNetworkIme ime)
        {
            mIme = ime;
            InitializeComponent();
        }

        public int sendAlipayCodes(string[] lines)
        {
            var builder = new StringBuilder("ALIPAY");
            int count = 0;

            foreach (var line in lines)
            {
                var code = line.Trim();
                if (code.Length > 0)
                {
                    builder.Append(' ').Append(code);
                    count++;
                }
            }

            if (count == 0)
            {
                return 0;
            }

            return mIme.sendCommand(builder.ToString(), true);
        }

        private void buttonOpen_Click(object sender, EventArgs e)
        {
            mIme.sendOpenApp("com.eg.android.AlipayGphone", true);
        }

        private void buttonSend_Click(object sender, EventArgs e)
        {
            sendAlipayCodes(textBoxCodes.Lines);
        }

        private void buttonClear_Click(object sender, EventArgs e)
        {
            mIme.sendCommand("CLEAR", true);
            textBoxCodes.Clear();
        }

        public void setClipboardText(string text)
        {
            textBoxCodes.Text = text;
        }

        public bool onClipboardChanged(string text)
        {
            if (IsDisposed)
            {
                return false;
            }

            if (checkBoxClipboardListen.Checked)
            {
                textBoxCodes.Text = text;
            }

            if (checkBoxClipboardSend.Checked)
            {
                var lines = text.Split('\n');
                sendAlipayCodes(lines);
                return true;
            }

            return false;
        }
    }
}
