using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace NetworkInputMethod
{
    public partial class FormDaemonEditor : Form
    {
        public FormDaemonEditor()
        {
            InitializeComponent();
        }

        public string Command { get => textBoxCommand.Text; set => textBoxCommand.Text = value; }

        public string Args { get => textBoxArgs.Text; set => textBoxArgs.Text = value; }

        private void buttonOpen_Click(object sender, EventArgs e)
        {
            if (openFileDialogCommand.ShowDialog() == DialogResult.OK)
            {
                textBoxCommand.Text = openFileDialogCommand.FileName;
            }
        }

        private void buttonOK_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(textBoxCommand.Text))
            {
                MessageBox.Show("请选择命令路径");
            }
            else
            {
                DialogResult = DialogResult.OK;
            }
        }
    }
}
