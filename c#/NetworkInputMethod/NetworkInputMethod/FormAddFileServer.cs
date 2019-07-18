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
    public partial class FormAddFileServer : Form
    {
        public FormAddFileServer()
        {
            InitializeComponent();
            comboBoxProtocl.SelectedIndex = 0;
        }

        public String Url
        {
            get
            {
                var builder = new StringBuilder();

                builder.Append(comboBoxProtocl.Text).Append("://");

                var user = textBoxUser.Text;

                if (user.Length > 0)
                {
                    builder.Append(user);

                    var pass = textBoxPass.Text;
                    if (pass.Length > 0)
                    {
                        builder.Append(':').Append(pass);
                    }

                    builder.Append('@');
                }

                var host = textBoxHost.Text;

                if (host.Length > 0)
                {
                    builder.Append(host);
                }
                else
                {
                    builder.Append("127.0.0.1");
                }

                var port = textBoxPort.Text;

                if (port.Length > 0)
                {
                    builder.Append(':').Append(port);
                }

                var cert = textBoxCert.Text;

                if (cert.Length > 0)
                {
                    builder.Append('/');
                    builder.Append(cert[0]);
                    builder.Append(cert.Substring(2).Replace('\\', '/'));
                }

                return builder.ToString();
            }
        }

        private void buttonCert_Click(object sender, EventArgs e)
        {
            if (openFileDialogCert.ShowDialog() == DialogResult.OK)
            {
                textBoxCert.Text = openFileDialogCert.FileName;
            }
        }
    }
}
