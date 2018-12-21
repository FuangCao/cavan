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

        public List<string> getAlipayCodes()
        {
            List<string> codes = new List<string>();

            foreach (var line in textBoxCodes.Lines)
            {
                var code = line.Trim();
                if (code.Length > 0)
                {
                    codes.Add(code);
                }
            }

            return codes;
        }

        private void buttonOpen_Click(object sender, EventArgs e)
        {
            mIme.sendOpenApp("com.eg.android.AlipayGphone");
        }

        private void buttonSend_Click(object sender, EventArgs e)
        {
            var codes = getAlipayCodes();
            if (codes.Count > 0)
            {
                var builder = new StringBuilder("ALIPAY");

                foreach (var code in codes)
                {
                    builder.Append(' ').Append(code);
                }

                mIme.sendCommand(builder.ToString(), true);
            }
        }

        private void buttonClear_Click(object sender, EventArgs e)
        {
            mIme.sendCommand("CLEAR", true);
        }
    }
}
