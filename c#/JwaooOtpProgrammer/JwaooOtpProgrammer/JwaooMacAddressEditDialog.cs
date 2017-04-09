using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace JwaooOtpProgrammer {
    public partial class JwaooMacAddressEditDialog : Form {

        private JwaooMacAddress mMacAddress;

        public JwaooMacAddressEditDialog(JwaooMacAddress address) {
            InitializeComponent();

            mMacAddress = address;
            textBoxAddressStart.Text = address.ToString();
            textBoxAddressCount.Text = address.AddressCountText;
        }

        private void buttonOK_Click(object sender, EventArgs e) {
            try {
                UInt32 count = Convert.ToUInt32(textBoxAddressCount.Text);

                CavanMacAddress address = new CavanMacAddress().fromString(textBoxAddressStart.Text);
                if (mMacAddress.isAddressValid(address)) {
                    mMacAddress.AddressCount = count;
                    mMacAddress.copyFrom(address);

                    DialogResult = DialogResult.OK;
                    Close();
                } else {
                    MessageBox.Show("MAC地址格式错误，应该类似于：" + mMacAddress.AddressStart);
                }
            } catch (Exception) {
                MessageBox.Show("格式错误，请重新输入！");
            }
            
        }
    }
}
