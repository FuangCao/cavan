using System;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using NetworkInputMethod.Properties;

namespace NetworkInputMethod
{
    public partial class FormCertManager : Form
    {
        public FormCertManager()
        {
            InitializeComponent();

            var certs = Settings.Default.HttpCaptureCerts;

            if (certs != null)
            {
                foreach (var node in certs)
                {
                    var args = node.Split('|');
                    if (args.Length < 3)
                    {
                        continue;
                    }

                    var item = listViewCerts.Items.Add(args[0]);
                    item.SubItems.Add(args[1]);
                    item.SubItems.Add(args[2]);
                }
            }
        }

        private void buttonAdd_Click(object sender, EventArgs e)
        {
            var domain = textBoxDomain.Text;
            if (string.IsNullOrEmpty(domain))
            {
                MessageBox.Show("请输入域名");
                return;
            }

            var password = textBoxPassword.Text;
            if (string.IsNullOrEmpty(password))
            {
                MessageBox.Show("请输入密码");
                return;
            }

            if (openFileDialogCert.ShowDialog() == DialogResult.OK)
            {
                var item = listViewCerts.Items.Add(domain);
                item.SubItems.Add(password);
                item.SubItems.Add(openFileDialogCert.FileName);
            }
        }

        private void buttonOK_Click(object sender, EventArgs e)
        {
            var lines = new StringCollection();

            foreach (ListViewItem item in listViewCerts.Items)
            {
                var items = item.SubItems;
                lines.Add(items[0].Text + "|" + items[1].Text + "|" + items[2].Text);
            }

            Settings.Default.HttpCaptureCerts = lines;
            Settings.Default.Save();
        }
    }
}
