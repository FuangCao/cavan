using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace JwaooOtpProgrammer {
    public partial class CavanMacAddressAllocAvgDialog : Form {

        private UInt32 mCount;
        private UInt32 mCountMax;

        public CavanMacAddressAllocAvgDialog(UInt32 max) {
            InitializeComponent();
            mCountMax = max;
            textBoxComputerCount.Text = "1";
        }

        public UInt32 Count {
            get {
                return mCount;
            }
        }

        private void buttonOK_Click(object sender, EventArgs e) {
            try {
                UInt32 count = Convert.ToUInt32(textBoxComputerCount.Text);
                if (count > 0) {
                    if (count > mCountMax) {
                        MessageBox.Show("电脑台数太多，请重新输入！");
                    } else {
                        DialogResult = DialogResult.OK;
                        mCount = count;
                        Close();
                    }
                } else {
                    MessageBox.Show("电脑台数太少，请重新输入！");
                }
            } catch (Exception) {
                MessageBox.Show("格式错误，请重新输入！");
            }
        }
    }
}
