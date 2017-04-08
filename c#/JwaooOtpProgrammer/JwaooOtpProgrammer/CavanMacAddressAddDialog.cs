using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace JwaooOtpProgrammer {
    public partial class CavanMacAddressAddDialog : Form {

        private UInt32 mAddressCount;
        private UInt32 mAddressCountMax;

        public CavanMacAddressAddDialog() {
            InitializeComponent();
        }

        public UInt32 AddressCount {
            get {
                return mAddressCount;
            }

            set {
                textBoxAddressCount.Text = Convert.ToString(value);
            }
        }

        public UInt32 AddressCountMax {
            get {
                return mAddressCountMax;
            }

            set {
                mAddressCountMax = value;
                DialogResult = DialogResult.Cancel;

                if (mAddressCount == 0 || mAddressCount > mAddressCountMax) {
                    mAddressCount = mAddressCountMax;
                }

                AddressCount = mAddressCount;
            }
        }

        private void buttonOK_Click(object sender, EventArgs e) {
            try {
                UInt32 count = Convert.ToUInt32(textBoxAddressCount.Text);

                if (count > 0) {
                    if (count > mAddressCountMax) {
                        MessageBox.Show("输入的地址数过大，请重新输入！");
                    } else {
                        DialogResult = DialogResult.OK;
                        mAddressCount = count;
                        Close();
                    }
                } else {
                    MessageBox.Show("输入的地址数过小，请重新输入！");
                }
            } catch {
                MessageBox.Show("格式错误，请重新输入！");
            }
        }
    }
}
