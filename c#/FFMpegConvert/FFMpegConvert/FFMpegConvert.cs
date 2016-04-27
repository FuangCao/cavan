using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace FFMpegConvert
{
    public partial class FFMpegConvert : Form
    {
        private Thread mConvertThread;

        public FFMpegConvert()
        {
            InitializeComponent();

            comboBoxVideoCodec.SelectedIndex = 0;
            comboBoxVideoBitRate.SelectedIndex = 0;
            comboBoxAudioCodec.SelectedIndex = 0;
            comboBoxAudioBitRate.SelectedIndex = 0;
        }

        private void textBoxInputDir_Click(object sender, EventArgs e)
        {
            if (textBoxInputDir.Text.Length > 0)
            {
                folderBrowserDialogInputDir.SelectedPath = textBoxInputDir.Text;
            }

            if (folderBrowserDialogInputDir.ShowDialog() == DialogResult.OK)
            {
                textBoxInputDir.Text = folderBrowserDialogInputDir.SelectedPath;

                if (textBoxOutputDir.Text.Length > 0)
                {
                    buttonStart.Enabled = true;
                }
            }
        }

        private void textBoxOutputDir_Click(object sender, EventArgs e)
        {
            if (textBoxOutputDir.Text.Length > 0)
            {
                folderBrowserDialogOutputDir.SelectedPath = textBoxOutputDir.Text;
            }

            if (folderBrowserDialogOutputDir.ShowDialog() == DialogResult.OK)
            {
                textBoxOutputDir.Text = folderBrowserDialogOutputDir.SelectedPath;

                if (textBoxInputDir.Text.Length > 0)
                {
                    buttonStart.Enabled = true;
                }
            }
        }

        private bool doConvert(FileInfo info)
        {
            Process process = null;

            Console.WriteLine("FullName = " + info.FullName);

            try
            {
                process = Process.Start("ffmpeg.exe");
                process.WaitForExit();
                return process.ExitCode == 0;
            }
            catch (Exception)
            {
                return false;
            }
            finally
            {
                if (process != null)
                {
                    process.Close();
                }
            }
        }

        private int doConvert(DirectoryInfo infoParent)
        {
            int count = 0;

            foreach (FileInfo info in infoParent.GetFiles())
            {
                if (!doConvert(info))
                {
                    count++;
                }
            }

            foreach (DirectoryInfo info in infoParent.GetDirectories())
            {
                count += doConvert(info);
            }

            return count;
        }

        delegate void SetConvertStateCallback(bool running);

        private void setConvertState(bool running)
        {
            if (running)
            {
                buttonStart.Enabled = false;
                buttonStop.Enabled = true;
            }
            else
            {
                buttonStart.Enabled = true;
                buttonStop.Enabled = false;
            }
        }

        private void doConvert()
        {
            int count = -1;
            String pathname = textBoxInputDir.Text;

            SetConvertStateCallback callback = new SetConvertStateCallback(setConvertState);
            Invoke(callback, new object[] { true });

            if (Directory.Exists(pathname))
            {
                count = doConvert(new DirectoryInfo(pathname));
            }
            else if (File.Exists(pathname) && doConvert(new FileInfo(pathname)))
            {
                count = 0;
            }

            Invoke(callback, new object[] { false });

            mConvertThread = null;
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            if (mConvertThread == null)
            {
                mConvertThread = new Thread(new ThreadStart(doConvert));
                mConvertThread.Start();
            }
        }

        private void buttonStop_Click(object sender, EventArgs e)
        {

        }
    }
}
