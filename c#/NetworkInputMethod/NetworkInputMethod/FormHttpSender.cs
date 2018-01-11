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
        private const long START_AHEAD = 15000;

        private StringBuilder mLogBuilder = new StringBuilder();
        private CavanHttpSender mSender;
        private long mCommitTime;
        private bool mSendEnabled;

        public FormHttpSender()
        {
            InitializeComponent();

            mSender = new CavanHttpSender(this);

            textBoxPath.Text = openFileDialogReq.FileName;

            DateTime time = DateTime.Now.AddHours(1);
            dateTimePickerStart.Value = new DateTime(time.Year, time.Month, time.Day, time.Hour, 0, 0);
            dateTimePickerStart.Format = DateTimePickerFormat.Custom;
            dateTimePickerStart.CustomFormat = "yyyy-MM-dd HH:mm:ss";

            comboBoxDelay.SelectedIndex = 30;
        }

        public bool SendEnabled
        {
            get
            {
                lock (this)
                {
                    return mSendEnabled;
                }
            }
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

        public bool isDebugEnabled()
        {
            return checkBoxDebug.Checked;
        }

        private void onLogUpdated(object sender, EventArgs e)
        {
            lock (mLogBuilder)
            {
                textBoxLog.Text = mLogBuilder.ToString();
            }
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

            mSender.Start();
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            buttonStart.Enabled = false;
            buttonStop.Enabled = false;

            lock (this)
            {
                if (mSendEnabled)
                {
                    startLocked(0);
                }
                else
                {
                    mSendEnabled = true;

                    if (mSender.parseFile(openFileDialogReq.FileName))
                    {
                        mCommitTime = dateTimePickerStart.Value.ToFileTime() / 10000 + getStartDelay();
                        timerWait.Interval = 100;
                        timerWait.Enabled = true;
                        labelStatus.Text = "正在等待";
                    }
                }
            }

            buttonStop.Enabled = true;
            buttonStart.Enabled = true;
        }

        private void buttonStop_Click(object sender, EventArgs e)
        {
            buttonStart.Enabled = false;
            buttonStop.Enabled = false;

            lock (this)
            {
                mSendEnabled = false;
                mCommitTime = 0;
                timerWait.Enabled = false;
            }

            mSender.Stop();

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
            if (mCommitTime != 0 && mSendEnabled)
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
