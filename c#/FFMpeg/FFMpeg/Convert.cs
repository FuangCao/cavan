using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace FFMpeg
{
    public partial class Convert : Form
    {
        public Convert()
        {
            InitializeComponent();

            comboBoxVideoCodec.SelectedIndex = 0;
            comboBoxVideoBitRate.SelectedIndex = 0;
            comboBoxAudioCodec.SelectedIndex = 0;
            comboBoxAudioBitRate.SelectedIndex = 0;
        }

        private void textBoxInputDir_Click(object sender, EventArgs e)
        {
            if (textBoxInputDir.Text.Length > 0) {
                folderBrowserDialogInputDir.SelectedPath = textBoxInputDir.Text;
            }

            if (folderBrowserDialogInputDir.ShowDialog() == DialogResult.OK) {
                textBoxInputDir.Text = folderBrowserDialogInputDir.SelectedPath;

                if (textBoxOutputDir.Text.Length > 0) {
                    buttonStart.Enabled = true;
                }
            }
        }

        private void textBoxOutputDir_Click(object sender, EventArgs e)
        {
            if (textBoxOutputDir.Text.Length > 0) {
                folderBrowserDialogOutputDir.SelectedPath = textBoxOutputDir.Text;
            }

            if (folderBrowserDialogOutputDir.ShowDialog() == DialogResult.OK) {
                textBoxOutputDir.Text = folderBrowserDialogOutputDir.SelectedPath;

                if (textBoxInputDir.Text.Length > 0) {
                    buttonStart.Enabled = true;
                }
            }
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {

        }

        private void buttonStop_Click(object sender, EventArgs e)
        {

        }
    }
}
