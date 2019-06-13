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
    public partial class FormMmMulti : Form
    {
        public FormMmMulti()
        {
            InitializeComponent();
        }

        public int Count {
            get {
                try
                {
                    return Convert.ToInt32(textBoxCount.Text);
                }
                catch (Exception)
                {
                    return 0;
                }
            }
        }
    }
}
