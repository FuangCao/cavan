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
    public partial class FormTextEditor : Form
    {
        public FormTextEditor(string title, string value)
        {
            InitializeComponent();
            this.Text = title;
            textBox.Text = value;
        }

        public string Value
        {
            get
            {
                return textBox.Text;
            }

            set
            {
                textBox.Text = value;
            }
        }
    }
}
