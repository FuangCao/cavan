using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Net.Sockets;
using System.Net.Security;
using System.Threading;
using System.Diagnostics;

namespace NetworkInputMethod
{
    public partial class FormHttpSender : Form
    {
        private const long START_AHEAD = 20000;

        private List<CavanHttpSender> mSenders = new List<CavanHttpSender>();
        private StringBuilder mLogBuilder = new StringBuilder();
        private long mCommitTime;

        public FormHttpSender()
        {
            InitializeComponent();

            if (File.Exists(openFileDialogReq.FileName))
            {
                textBoxPath.Text = openFileDialogReq.FileName;
            }

            DateTime tomorrow = DateTime.Now.AddDays(1);
            DateTime time = new DateTime(tomorrow.Year, tomorrow.Month, tomorrow.Day, 0, 0, 0);
            dateTimePickerStart.Value = time;
            dateTimePickerStart.Format = DateTimePickerFormat.Custom;
            dateTimePickerStart.CustomFormat = "yyyy-MM-dd HH:mm:ss";

            comboBoxDelay.SelectedIndex = 30;
        }

        public int getStartDelay()
        {
            int index = comboBoxDelay.SelectedIndex;
            if (index < 0)
            {
                return 0;
            }

            return (index - 30) * 100;
        }

        public long getSendDelay()
        {
            long timeNow = DateTime.Now.ToFileTime() / 10000;
            if (mCommitTime > timeNow)
            {
                return mCommitTime - timeNow;
            }

            return 0;
        }

        public void waitForSend()
        {
            while (true)
            {
                long delay = getSendDelay();
                if (delay > 0)
                {
                    WriteLog("等待发送：" + delayToString(delay));
                    Thread.Sleep((int)delay);
                }
                else
                {
                    WriteLog("开始发送");
                    break;
                }
            }
        }

        public void WriteLog(string log)
        {
            lock (mLogBuilder)
            {
                mLogBuilder.Append(log);
                mLogBuilder.Append("\r\n");
            }

            EventHandler handler = new EventHandler(onLogUpdated);
            textBoxLog.Invoke(handler);
        }

        private void onLogUpdated(object sender, EventArgs e)
        {
            lock (mLogBuilder)
            {
                textBoxLog.Text = mLogBuilder.ToString();
            }
        }

        private bool parseLocked()
        {
            string filename = openFileDialogReq.FileName;
            if (filename == null || filename.Length == 0)
            {
                MessageBox.Show("请选择请求文件！");
                return false;
            }

            string[] lines;

            try
            {
                lines = File.ReadAllLines(filename);
            }
            catch (Exception)
            {
                lines = null;
            }

            if (lines == null)
            {
                MessageBox.Show("无法读取文件: " + filename);
                return false;
            }

            CavanHttpSender sender = new CavanHttpSender(this);

            foreach (string line in lines)
            {
                if (sender.addLine(line))
                {
                    mSenders.Add(sender);
                    sender = new CavanHttpSender(this);
                }
            }

            if (sender.LineCount > 0)
            {
                mSenders.Add(sender);
            }

            if (mSenders.Count == 0)
            {
                MessageBox.Show("没有发现HTTP请求！");
                return false;
            }

            WriteLog("发现 " + mSenders.Count + " 个请求");

            return true;
        }

        private void startLocked(long delay)
        {
            if (delay > 0)
            {
                labelStatus.Text = "等待发送：" + delayToString(delay);

                if (delay > 1000)
                {
                    delay = 1000;
                }

                timerWait.Interval = (int)delay;
            }
            else
            {
                mCommitTime = 0;
                timerWait.Enabled = false;
                labelStatus.Text = "正在运行";
            }

            foreach (CavanHttpSender http in mSenders)
            {
                http.start();
            }
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            buttonStart.Enabled = false;
            buttonStop.Enabled = false;

            Monitor.Enter(mSenders);

            if (mSenders.Count > 0)
            {
                startLocked(0);
            }
            else if (parseLocked())
            {
                mCommitTime = dateTimePickerStart.Value.ToFileTime() / 10000 + getStartDelay();
                timerWait.Interval = 100;
                timerWait.Enabled = true;
                labelStatus.Text = "正在等待";
            }

            Monitor.Exit(mSenders);

            buttonStop.Enabled = true;
            buttonStart.Enabled = true;
        }

        private void buttonStop_Click(object sender, EventArgs e)
        {
            buttonStart.Enabled = false;
            buttonStop.Enabled = false;

            lock (mSenders)
            {
                mCommitTime = 0;
                timerWait.Enabled = false;

                foreach (CavanHttpSender http in mSenders)
                {
                    http.stop();
                }

                mSenders.Clear();
            }

            labelStatus.Text = "已停止运行";

            buttonStop.Enabled = true;
            buttonStart.Enabled = true;

        }

        private void buttonOpen_Click(object sender, EventArgs e)
        {
            if (openFileDialogReq.ShowDialog() == DialogResult.OK)
            {
                textBoxPath.Text = openFileDialogReq.FileName;
            }
        }

        private void buttonClear_Click(object sender, EventArgs e)
        {
            lock (mLogBuilder)
            {
                mLogBuilder.Length = 0;
                textBoxLog.Text = mLogBuilder.ToString();
            }
        }

        private string delayToString(long delay)
        {
            StringBuilder builder = new StringBuilder();

            if (delay >= 86400000)
            {
                builder.Append(delay / 86400000).Append("天");
                delay %= 86400000;
            }

            if (delay >= 3600000)
            {
                builder.Append(delay / 3600000).Append("小时");
                delay %= 3600000;
            }

            if (delay >= 60000)
            {
                builder.Append(delay / 60000).Append("分");
                delay %= 60000;
            }

            builder.Append((float)delay / 1000).Append("秒");

            return builder.ToString();
        }

        private void timerWait_Tick(object sender, EventArgs e)
        {
            lock (mSenders)
            {
                if (mCommitTime != 0 && mSenders.Count > 0)
                {
                    long delay = getSendDelay();
                    if (delay > START_AHEAD)
                    {
                        labelStatus.Text = "等待启动：" + delayToString(delay);
                        timerWait.Interval = 1000;
                    }
                    else
                    {
                        startLocked(delay);
                    }
                }
                else
                {
                    timerWait.Enabled = false;
                }
            }
        }

        private void buttonNow_Click(object sender, EventArgs e)
        {
            dateTimePickerStart.Value = DateTime.Now;
        }

        private void buttonEdit_Click(object sender, EventArgs e)
        {
            Process.Start("notepad.exe", openFileDialogReq.FileName);
        }
    }
}
