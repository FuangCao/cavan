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
using System.Configuration;
using System.IO;

namespace RedPacketListener {

    public partial class Form1 : Form {

        public const string RED_PACKET_PREFIX = "RedPacketCode: ";
        public static Regex[] sRedPackageRegexs = {
            new Regex("支\\s*付\\s*宝.*口\\s*令\\s*[:：]?([\\s\\d]+)"),
            new Regex("支\\s*付\\s*宝.*红\\s*包\\s*[:：]?([\\s\\d]+)"),
            new Regex("口\\s*令.*红\\s*包\\s*[:：]?([\\s\\d]+)"),
            new Regex("红\\s*包.*口\\s*令\\s*[:：]?([\\s\\d]+)"),
            new Regex("红\\s*包\\s*[:：]([\\s\\w]+)"),
            new Regex("口\\s*令\\s*[:：]([\\s\\w]+)"),
        };

        private delegate string GetClipboardTextCallback();
        private delegate void OnClipboardChangedCallback(string text);
        private delegate void UpdateConnectStateCallback(int state);
        private delegate void OnRedPacketCodeReceivedCallback(string code);

        private Thread mClipThread;
        private bool mClipThreadRunning;

        private UdpClient mUdpClient;
        private NetworkStream mStream;
        private Thread mNetworkThread;
        private bool mNetworkThreadRunning;

        public Form1() {
            InitializeComponent();
            loadConfig();
            mUdpClient = new UdpClient("224.0.0.1", 9898);
        }

        ~Form1() {
            mUdpClient.Close();
            disconnect();
        }

        private void loadConfig() {
            textBoxIp.Text = ConfigurationManager.AppSettings["IP"];
            textBoxPort.Text = ConfigurationManager.AppSettings["Port"];
        }

        private bool saveConfig() {
            try {
                Configuration config = ConfigurationManager.OpenExeConfiguration(Application.ExecutablePath);
                if (config == null) {
                    return false;
                }

                KeyValueConfigurationCollection collection = config.AppSettings.Settings;
                collection.Clear();
                collection.Add("IP", textBoxIp.Text);
                collection.Add("Port", textBoxPort.Text);

                config.Save(ConfigurationSaveMode.Full);

                return true;
            } catch {
                return false;
            }
        }

        private string getClipboardText() {
            string clip = Clipboard.GetText();
            if (clip == null) {
                return "";
            }

            return clip;
        }

        private void updateConnectState(int state) {
            if (state > 0) {
                buttonDisconnect.Enabled = true;
                buttonConnect.Enabled = false;
                buttonSend.Enabled = true;
                buttonTest.Enabled = true;
                textBoxIp.Enabled = false;
                textBoxPort.Enabled = false;

                labelState.Text = "已连接";
                saveConfig();

                mClipThreadRunning = true;

                if (mClipThread == null) {
                    mClipThread = new Thread(new ThreadStart(ClipPollThread));
                    mClipThread.Start();
                }
            } else {
                if (state < 0) {
                    buttonDisconnect.Enabled = false;
                    buttonConnect.Enabled = false;
                    buttonSend.Enabled = false;
                    buttonTest.Enabled = false;
                    textBoxIp.Enabled = false;
                    textBoxPort.Enabled = false;

                    labelState.Text = "正在连接";
                } else {
                    buttonDisconnect.Enabled = false;
                    buttonConnect.Enabled = true;
                    buttonSend.Enabled = false;
                    buttonTest.Enabled = false;
                    textBoxIp.Enabled = true;
                    textBoxPort.Enabled = true;

                    labelState.Text = "连接已断开";
                }
            }
        }

        private void ClipPollThread() {
            GetClipboardTextCallback callback = new GetClipboardTextCallback(getClipboardText);
            OnClipboardChangedCallback changed = new OnClipboardChangedCallback(onClipboardChanged);

            string backup = (string) Invoke(callback);

            mClipThreadRunning = true;

            while (mClipThreadRunning) {
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

            mClipThreadRunning = false;
            mClipThread = null;
        }

        private void disconnect() {
            mNetworkThreadRunning = false;

            if (mStream != null) {
                mStream.Close();
            }

            mClipThreadRunning = false;
        }

        private void onRedPacketCodeReceived(string code) {
            listBoxCodes.Items.Add(code);
        }

        private void NetworkReceiveThread() {
            UpdateConnectStateCallback callback = new UpdateConnectStateCallback(updateConnectState);
            OnRedPacketCodeReceivedCallback received = new OnRedPacketCodeReceivedCallback(onRedPacketCodeReceived);

            mNetworkThreadRunning = true;

            while (mNetworkThreadRunning) {
                TcpClient client = null;

                Invoke(callback, new object[] { -1 });

                try {
                    client = new TcpClient(textBoxIp.Text, int.Parse(textBoxPort.Text));
                } catch {
                    break;
                }

                if (client == null) {
                    break;
                }

                NetworkStream stream = null;
                StreamReader reader = null;

                try {
                    stream = client.GetStream();
                    reader = new StreamReader(stream);

                    mStream = stream;

                    Invoke(callback, new object[] { 1 });

                    while (true) {
                        string line = reader.ReadLine();
                        if (line == null) {
                            break;
                        }

                        if (line.StartsWith(RED_PACKET_PREFIX)) {
                            Invoke(received, new object[] { line.Substring(RED_PACKET_PREFIX.Length).Trim() });
                        }
                    }
                } catch {
                }

                mStream = null;

                if (reader != null) {
                    reader.Close();
                }

                if (stream != null) {
                    stream.Close();
                }

                client.Close();
            }

            try {
                Invoke(callback, new object[] { 0 });
            } catch {
            }

            mNetworkThread = null;
            mNetworkThreadRunning = false;
        }

        private bool sendRedPacketCode(String code) {
            try {
                byte[] bytes = Encoding.UTF8.GetBytes(RED_PACKET_PREFIX + code + "\n");

                mUdpClient.Send(bytes, bytes.Length - 1);

                if (mStream != null) {
                    mStream.Write(bytes, 0, bytes.Length);
                    listBoxCodes.Items.Add(code);
                }

                mUdpClient.Send(bytes, bytes.Length - 1);

                return true;
            } catch {
                return false;
            }
        }

        private void onClipboardChanged(string text) {
            List<string> codes = new List<string>();

            foreach (string line in text.Split(new char[] { '\n' })) {
                foreach (Regex regex in sRedPackageRegexs) {
                    Match match = regex.Match(line);

                    while (match != null && match.Success) {
                        string code = Regex.Replace(match.Groups[1].Value, "\\s+", "");
                        if (codes.IndexOf(code) < 0) {
                            codes.Add(code);
                        }

                        match = match.NextMatch();
                    }
                }
            }

            foreach (string code in codes) {
                sendRedPacketCode(code);
            }
        }

        private void buttonConnect_Click(object sender, EventArgs e) {
            mNetworkThreadRunning = true;

            if (mNetworkThread == null) {
                mNetworkThread = new Thread(new ThreadStart(NetworkReceiveThread));
                mNetworkThread.Start();
            }
        }

        private bool sendTextBoxCode() {
            string text = textBoxSend.Text;
            if (text == null) {
                return false;
            }

            string code = Regex.Replace(text, "\\W+", "");
            if (code.Length > 0) {
                sendRedPacketCode(code);
                return true;
            }

            return false;
        }

        private void textBoxSend_KeyPress(object sender, KeyPressEventArgs e) {
            if (e.KeyChar == (char) Keys.Enter) {
                sendTextBoxCode();
            }
        }

        private void buttonSend_Click(object sender, EventArgs e) {
            if (!sendTextBoxCode()) {
                onClipboardChanged(getClipboardText());
            }
        }

        private void buttonClear_Click(object sender, EventArgs e) {
            listBoxCodes.Items.Clear();
        }

        private void listBoxCodes_DoubleClick(object sender, EventArgs e) {
            object item = listBoxCodes.SelectedItem;
            if (item != null) {
                sendRedPacketCode(item.ToString());
            }
        }

        private void buttonDisconnect_Click(object sender, EventArgs e) {
            disconnect();
        }

        private void buttonTest_Click(object sender, EventArgs e) {
            sendRedPacketCode("CavanNetworkTest");
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e) {
            disconnect();
        }

        private void listBoxCodes_SelectedIndexChanged(object sender, EventArgs e) {
            object item = listBoxCodes.SelectedItem;
            if (item != null) {
                textBoxSend.Text = item.ToString();
            }
        }
    }
}
