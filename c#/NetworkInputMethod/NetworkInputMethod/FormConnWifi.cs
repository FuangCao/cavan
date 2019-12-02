using NetworkInputMethod.Properties;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace NetworkInputMethod
{
    public partial class FormConnWifi : Form
    {
        private FormNetworkIme mNetworkIme;

        public FormConnWifi(FormNetworkIme ime)
        {
            mNetworkIme = ime;
            InitializeComponent();

            var wifis = Settings.Default.Wifis;
            if (wifis != null)
            {
                foreach (var wifi in wifis)
                {
                    listBoxWifis.Items.Add(wifi);
                }
            }
        }

        public void SaveWifis()
        {
            StringCollection wifis = new StringCollection();

            foreach (var item in listBoxWifis.Items)
            {
                wifis.Add(item.ToString());
            }

            Settings.Default.Wifis = wifis;
            Settings.Default.Save();
        }

        private void toolStripMenuItemConn_Click(object sender, EventArgs e)
        {
            var item = listBoxWifis.SelectedItem;
            if (item != null)
            {
                mNetworkIme.sendCommand("WIFI " + item, true);
            }
            else
            {
                mNetworkIme.sendCommand("WIFI", true);
            }
        }

        private void toolStripMenuItemAdd_Click(object sender, EventArgs e)
        {
            var form = new FormTextEditor("添加WiFi", null);
            if (form.ShowDialog() == DialogResult.OK)
            {
                listBoxWifis.Items.Add(form.Value);
                SaveWifis();
            }
        }

        private void toolStripMenuItemDel_Click(object sender, EventArgs e)
        {
            var item = listBoxWifis.SelectedItem;
            if (item != null)
            {
                listBoxWifis.Items.Remove(item);
                SaveWifis();
            }
        }

        private void contextMenuStripWifis_Opening(object sender, CancelEventArgs e)
        {
            toolStripMenuItemDel.Enabled = (listBoxWifis.SelectedItem != null);
        }
    }
}
