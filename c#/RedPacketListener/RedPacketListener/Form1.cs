using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;
using System.Text.RegularExpressions;
using System.Threading;

namespace RedPacketListener {

    public partial class Form1 : Form {

        public static Regex sRedPackageRegex = new Regex("支付宝.*口令.*[:：]([\\s\\w]+)");

        private delegate string GetClipboardTextCallback();
        private delegate void OnClipboardChangedCallback(string text);

        private TcpClient mClient;
        private Thread mThread;

        public Form1() {
            InitializeComponent();
        }

        private string getClipboardText() {
            string clip = Clipboard.GetText();
            if (clip == null) {
                return "";
            }

            return clip;
        }

        private void clipPollThreadHandler() {
            GetClipboardTextCallback callback = new GetClipboardTextCallback(getClipboardText);
            OnClipboardChangedCallback changed = new OnClipboardChangedCallback(onClipboardChanged);

            string backup = (string) Invoke(callback);

            while (mClient != null) {
                try {
                    string text = (string)Invoke(callback);
                    if (text.Equals(backup) == false) {
                        backup = text;
                        Invoke(changed, new object[] { text });
                    }
                } catch {
                    break;
                }

                Thread.Sleep(500);
            }

            mThread = null;
        }

        private Boolean sendRedPacketCode(String code) {
            if (mClient == null) {
                return false;
            }

            try {
                byte[] bytes = Encoding.Default.GetBytes("RedPacketCode: " + code + "\n");
                NetworkStream stream = mClient.GetStream();
                stream.Write(bytes, 0, bytes.Length);
            } catch {
                return false;
            }

            return true;
        }

        private void onClipboardChanged(string text) {
            foreach (string line in text.Split(new char[] { '\n' })) {
                Match match = sRedPackageRegex.Match(line);
                if (match != null && match.Success) {
                    string code = match.Groups[1].Value;
                    if (sendRedPacketCode(code)) {
                        listBoxCodes.Items.Add(code);
                    }
                }
            }
        }

        private void buttonConnect_Click(object sender, EventArgs e) {
            try {
                mClient = new TcpClient(textBoxIp.Text, int.Parse(textBoxPort.Text));
                if (mClient != null) {
                    labelState.Text = "已连接";

                    if (mThread == null) {
                        mThread = new Thread(new ThreadStart(clipPollThreadHandler));
                        mThread.Start();
                    }
                } else {
                    labelState.Text = "未连接";
                }
            } catch {
                mClient = null;
                labelState.Text = "连接失败";
            }
        }

        private void buttonSend_Click(object sender, EventArgs e) {
            onClipboardChanged(getClipboardText());
        }

        private void buttonClean_Click(object sender, EventArgs e) {
            listBoxCodes.Items.Clear();
        }

        private void listBoxCodes_DoubleClick(object sender, EventArgs e) {
            object item = listBoxCodes.SelectedItem;
            if (item != null) {
                sendRedPacketCode(listBoxCodes.SelectedItem.ToString());
            }
        }
    }
}
