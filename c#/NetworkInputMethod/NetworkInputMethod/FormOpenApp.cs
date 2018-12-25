using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;

namespace NetworkInputMethod
{
    public partial class FormOpenApp : Form
    {
        public static string[] sProgramFiles = {
            "C:\\Program Files (x86)",
            "C:\\Program Files",
            "D:\\Program Files (x86)",
            "D:\\Program Files",
        };

        private FormNetworkIme mNetworkIme;

        public FormOpenApp(FormNetworkIme ime)
        {
            mNetworkIme = ime;
            InitializeComponent();
        }

        private void buttonJxqy_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.jxqy");
        }

        private void buttonXxsy_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.xxsy");
        }

        private void buttonWec_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.wec");
        }

        private void buttonXymobile_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.xymobile");
        }

        private void buttonDragonnest_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.dragonnest");
        }

        private void buttonCf_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.cf");
        }

        private void buttonCldts_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.cldts");
        }

        private void buttonBayechuanqi_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.game37.bayechuanqi");
        }

        private void buttonDdtank_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.ddtank");
        }

        private void buttonSgame_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.sgame");
        }

        private void buttonNba_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.NBA");
        }

        private void buttonSupercellClashroyale_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.supercell.clashroyale");
        }

        private void buttonSupercellClashofclans_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.supercell.clashofclans");
        }

        private void buttonMm_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.mm");
        }

        private string getWeixinPath()
        {
            foreach (var dir in sProgramFiles)
            {
                if (Directory.Exists(dir))
                {
                    var path = Path.Combine(dir, "Tencent\\WeChat\\WeChat.exe");

                    if (File.Exists(path))
                    {
                        return path;
                    }
                }
            }

            if (openFileDialogWeChat.ShowDialog() == DialogResult.OK)
            {
                return openFileDialogWeChat.FileName;
            }

            return null;
        }

        private void buttonMmMulti_Click(object sender, EventArgs e)
        {
            FormMmMulti form = new FormMmMulti();
            if (form.ShowDialog() != DialogResult.OK)
            {
                return;
            }

            var path = getWeixinPath();
            if (path == null)
            {
                return;
            }

            Console.WriteLine("path = " + path);

            for (int i = form.Count; i > 0; i--)
            {
                Process process = new Process();
                process.StartInfo.FileName = path;
                process.Start();
            }
        }

        private void buttonBack_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendCommand("BACK", false);
        }

        private void buttonHome_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendCommand("HOME", false);
        }

        private void buttonTstl_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.tstl");
        }

        private void buttonPubgmhd_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.pubgmhd");
        }

        private void buttonCrossgate_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.crossgate");
        }

        private void buttonOpen_Click(object sender, EventArgs e)
        {
            var name = textBoxAppName.Text.Trim();
            if (name.Length > 0)
            {
                mNetworkIme.sendOpenApp(name);
            }
        }
    }
}
