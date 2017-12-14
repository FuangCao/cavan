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
    public partial class FormOpenApp : Form
    {
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
    }
}
