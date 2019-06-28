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
    public partial class FormAddProxy : Form
    {
        public FormAddProxy()
        {
            InitializeComponent();
        }

        public int Port
        {
            get
            {
                try
                {
                    return Convert.ToUInt16(textBoxPort.Text);
                }
                catch (Exception)
                {
                    return 0;
                }
            }

            set
            {
                textBoxPort.Text = value.ToString();
            }
        }

        public string ServerUrl
        {
            get
            {
                return textBoxServer.Text;
            }

            set
            {
                textBoxServer.Text = value;
            }
        }

        public string ClientAddress
        {
            get
            {
                return textBoxClientAddress.Text;
            }

            set
            {
                textBoxClientAddress.Text = value;
            }
        }

        public string ClientName
        {
            get
            {
                return textBoxClientName.Text;
            }

            set
            {
                textBoxClientName.Text = value;
            }
        }

        public string TargetUrl
        {
            get
            {
                return textBoxTarget.Text;
            }

            set
            {
                textBoxTarget.Text = value;
            }
        }
    }
}
