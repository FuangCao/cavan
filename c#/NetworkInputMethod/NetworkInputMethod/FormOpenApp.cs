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
            mNetworkIme.sendOpenApp("com.tencent.tmgp.jxqy", false);
        }

        private void buttonXxsy_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.xxsy", false);
        }

        private void buttonWec_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.wec", false);
        }

        private void buttonXymobile_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.xymobile", false);
        }

        private void buttonDragonnest_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.dragonnest", false);
        }

        private void buttonCf_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.cf", false);
        }

        private void buttonCldts_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.cldts", false);
        }

        private void buttonBayechuanqi_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.game37.bayechuanqi", false);
        }

        private void buttonDdtank_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.ddtank", false);
        }

        private void buttonSgame_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.sgame", false);
        }

        private void buttonNba_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.NBA", false);
        }

        private void buttonSupercellClashroyale_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.supercell.clashroyale", false);
        }

        private void buttonSupercellClashofclans_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.supercell.clashofclans", false);
        }

        private void buttonMm_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.mm", false);
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
            mNetworkIme.sendCommand("DESKTOP", false);
        }

        private void buttonTstl_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.tstl", false);
        }

        private void buttonPubgmhd_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.tmgp.pubgmhd", false);
        }

        private void buttonCrossgate_Click(object sender, EventArgs e)
        {
            mNetworkIme.sendOpenApp("com.tencent.crossgate", false);
        }

        private void buttonOpen_Click(object sender, EventArgs e)
        {
            var name = textBoxAppName.Text.Trim();
            if (name.Length > 0)
            {
                mNetworkIme.sendOpenApp(name, false);
            }
        }
    }
}
