using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;

namespace RedPacketListener {

    public partial class Form1 : Form {

        public static Regex sRedPackageRegex = new Regex("支付宝.*口令.*[:：]([\\s\\w]+)");

        private TcpClient mClient;

        public Form1() {
            InitializeComponent();
        }

        private void buttonConnect_Click(object sender, EventArgs e) {
            try {
                mClient = new TcpClient(textBoxIp.Text, int.Parse(textBoxPort.Text));
                if (mClient != null) {
                    labelState.Text = "已连接";
                } else {
                    labelState.Text = "未连接";
                }
            } catch {
                mClient = null;
                labelState.Text = "连接失败";
            }
        }

        /* [图片]
         * [图片]
         * [图片]
         * [图片]
         * [图片]
         * 支付宝红包口令：11223341
         * 支付宝红包口令：11223342
         * 支付宝红包口令：11223343
         * 支付宝红包口令：11223344
         * 支付宝红包口令：56418195
         */

        private void buttonSend_Click(object sender, EventArgs e) {
            string text = Clipboard.GetText();
            if (text != null) {
                foreach (string line in text.Split(new char[] { '\n' })) {
                    Match match = sRedPackageRegex.Match(line);
                    if (match != null && match.Success) {
                        string code = "RedPacketCode: " + match.Groups[1].Value;
                        labelState.Text = code;

                        if (mClient != null) {
                            byte[] bytes = Encoding.Default.GetBytes(code + "\n");
                            NetworkStream stream = mClient.GetStream();
                            stream.Write(bytes, 0, bytes.Length);
                        }
                    }
                }
            }
        }
    }
}
