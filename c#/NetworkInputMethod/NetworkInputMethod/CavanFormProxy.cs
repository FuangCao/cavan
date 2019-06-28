using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace NetworkInputMethod
{
    public class CavanSubForm : Form
    {
        public CavanSubForm()
        {
            FormClosing += onCavanSubFormClosing;
        }

        protected virtual void onCavanSubFormClosing(object sender, FormClosingEventArgs e)
        {
            Visible = false;
            e.Cancel = true;
        }

        public new bool Show()
        {
            if (base.IsDisposed)
            {
                return false;
            }

            base.Show();
            base.Focus();

            return true;
        }
    }
}
