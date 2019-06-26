using System;
using System.ComponentModel;
using System.Text;
using System.Windows.Forms;
using System.Net.Sockets;
using System.Threading;
using System.Runtime.InteropServices;
using System.Collections;
using System.Diagnostics;
using Microsoft.Win32;
using NetworkInputMethod.Properties;

namespace NetworkInputMethod
{
    public partial class FormNetworkIme : FormTcpService
    {
        private const int WM_DRAWCLIPBOARD = 0x308;
        private const int WM_CHANGECBCHAIN = 0x30D;

        private FormOpenApp mFormOpenApp;
        private FormSelect mFormSelect;
        private FormHttpSender mFormSender;
        private FormPackBuilder mFormBuilder;
        private FormAlipay mFormAlipay;
        private FormSendCommand mFormSendCommand;
        private FormTcpProxyService mFormTcpProxy;
        private FormWebProxyService mFormWebProxy;
        private FormUrlBuilder mFormUrlBuilder;

        //API declarations...
        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        static public extern IntPtr SetClipboardViewer(IntPtr hWndNewViewer);
        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        static public extern bool ChangeClipboardChain(IntPtr HWnd, IntPtr HWndNext);
        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        public static extern int SendMessage(IntPtr hWnd, int msg, IntPtr wParam, IntPtr lParam);

        private byte[] mRepeatSendBytes;
        private CavanTcpService mService;
        private IntPtr mNextClipboardViewer;
        private CavanBusyLock mBusyLock = new CavanBusyLock();
        private Thread mClockThread;

        private string mClipboardText = string.Empty;
        private long mClipboardTime;

        delegate void SimpleDelegate(Object obj);
        delegate void UpdateClockdelegate(DateTime date);

        public static bool setAutoRunEnable(bool enable)
        {
            var path = Application.ExecutablePath;
            var name = Application.ProductName;
            RegistryKey key = null;

            try
            {
                key = Registry.LocalMachine.OpenSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", true);

                if (enable)
                {
                    if (key == null)
                    {
                        key = Registry.LocalMachine.CreateSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
                        if (key == null)
                        {
                            return false;
                        }
                    }

                    key.SetValue(name, path);
                }
                else if (key != null)
                {
                    var value = key.GetValue(name);
                    if (value != null && value.Equals(path))
                    {
                        key.DeleteValue(name);
                    }
                }
            }
            catch (Exception e)
            {
                MessageBox.Show(e.ToString());
                return false;
            }
            finally
            {
                if (key != null)
                {
                    key.Close();
                }
            }

            return true;
        }

        public static bool isAutoRunEnabled()
        {
            RegistryKey key = null;

            try
            {
                key = Registry.LocalMachine.OpenSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", false);

                if (key == null)
                {
                    return false;
                }

                var name = Application.ProductName;

                var value = key.GetValue(name);
                if (value == null)
                {
                    return false;
                }

                return value.Equals(Application.ExecutablePath);
            }
            catch (Exception)
            {
                return false;
            }
            finally
            {
                if (key != null)
                {
                    key.Close();
                }
            }
        }

        public FormNetworkIme()
        {
            mClockThread = new Thread(new ThreadStart(ClockThreadHandler));
            mService = new CavanTcpService(this);

            InitializeComponent();
            textBoxPort.Text = Settings.Default.NetworkImePort.ToString();
            comboBoxSend.SelectedIndex = 0;
            comboBoxRepeat.SelectedIndex = 0;
        }

        private void FormNetworkIme_Load(object sender, EventArgs e)
        {
            mClockThread.Start();

            if (Settings.Default.NetworkImeEnable)
            {
                buttonStart.PerformClick();
            }

            if (Settings.Default.WebProxyEnable)
            {
                mFormWebProxy = new FormWebProxyService();
                mFormWebProxy.Show();
            }
        }

        private void UpdateClock(DateTime date)
        {
            textBoxClock.Text = string.Format("{0:F}.{1:D3}", date, date.Millisecond);
        }

        private void ClockThreadHandler()
        {
            int second = -1;

            while (true)
            {
                DateTime date = DateTime.Now;

                if (date.Second != second)
                {
                    var method = new UpdateClockdelegate(UpdateClock);
                    textBoxClock.Invoke(method, date);
                    second = date.Second;
                }

                Thread.Sleep(1000 - date.Millisecond);

                if (IsDisposed)
                {
                    break;
                }
            }
        }

        public void SetClipboardViewer()
        {
            IntPtr HWnd = Handle;

            if (mNextClipboardViewer != null)
            {
                ChangeClipboardChain(HWnd, mNextClipboardViewer);
            }

            mNextClipboardViewer = SetClipboardViewer(HWnd);
        }

        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            switch (keyData)
            {
                case Keys.Control | Keys.Enter:
                    if (checkBoxEnterSend.Checked)
                    {
                        break;
                    }
                    buttonSend.PerformClick();
                    return true;

                case Keys.Enter:
                    if (checkBoxEnterSend.Checked)
                    {
                        buttonSend.PerformClick();
                        return true;
                    }
                    break;

                case Keys.Escape:
                    buttonHome.PerformClick();
                    break;

                case Keys.F1:
                    buttonWeixin.PerformClick();
                    break;

                case Keys.F2:
                    buttonWeibo.PerformClick();
                    break;

                case Keys.F3:
                    buttonAlipay.PerformClick();
                    break;

                case Keys.F4:
                    buttonTaobao.PerformClick();
                    break;

                case Keys.F5:
                    buttonRefresh.PerformClick();
                    break;

                case Keys.F6:
                    buttonSignin.PerformClick();
                    break;

                case Keys.F7:
                    buttonUnfollow.PerformClick();
                    break;

                case Keys.F9:
                    buttonUnlock.PerformClick();
                    break;

                case Keys.F10:
                    buttonLock.PerformClick();
                    break;

                case Keys.F11:
                    buttonShareFriends.PerformClick();
                    break;

                case Keys.F12:
                    buttonShare.PerformClick();
                    break;
            }

            return base.ProcessCmdKey(ref msg, keyData);
        }

        protected override void WndProc(ref Message m)
        {
            switch (m.Msg)
            {
                case WM_DRAWCLIPBOARD:
                    onClipboardChanged();
                    SendMessage(mNextClipboardViewer, m.Msg, m.WParam, m.LParam);
                    break;

                case WM_CHANGECBCHAIN:
                    if (m.WParam == mNextClipboardViewer)
                    {
                        mNextClipboardViewer = m.LParam;
                    }
                    else
                    {
                        SendMessage(mNextClipboardViewer, m.Msg, m.WParam, m.LParam);
                    }
                    break;

                default:
                    base.WndProc(ref m);
                    break;
            }
        }

        public string getClipboardText()
        {
            try
            {
                var text = Clipboard.GetText();
                if (text == null)
                {
                    return null;
                }

                return text.Trim();
            }
            catch (Exception e)
            {
                MessageBox.Show(e.ToString());
            }

            return null;
        }

        private void onClipboardChanged()
        {
            string text = getClipboardText();

            if (string.IsNullOrEmpty(text))
            {
                return;
            }

            var time = DateTime.Now.ToFileTimeUtc();

            if (text.Equals(mClipboardText) && time - mClipboardTime < 10000000)
            {
                return;
            }

            Console.WriteLine("onClipboardChanged: " + text);

            mClipboardText = text;
            mClipboardTime = time;

            if (onClipboardChanged(text))
            {
                return;
            }

            if (checkBoxShareClipboard.Checked)
            {
                sendCommand("CLIPBOARD " + text, true);
            }

            if (checkBoxSendClipboard.Checked)
            {
                sendCommand("SEND " + text, false);
            }
        }

        private bool onClipboardChanged(string text)
        {
            if (mFormAlipay != null && mFormAlipay.Visible && mFormAlipay.onClipboardChanged(text))
            {
                return true;
            }

            if (mFormBuilder != null && mFormBuilder.Visible && mFormBuilder.postClipboard(text))
            {
                return true;
            }

            if (mFormUrlBuilder != null && mFormUrlBuilder.Visible && mFormUrlBuilder.postClipboard(text))
            {
                return true;
            }

            return false;
        }

        public int sendCommand(byte[] bytes, bool all)
        {
            int count = 0;
            ICollection coll;

            if (all)
            {
                coll = checkedListBoxClients.Items;
            }
            else
            {
                coll = checkedListBoxClients.CheckedItems;
            }

            foreach (Object item in coll)
            {
                NetworkImeClient client = item as NetworkImeClient;
                if (!client.send(bytes))
                {
                    count++;
                }
            }

            return count;
        }

        public ICollection NetworkImeClients
        {
            get
            {
                return checkedListBoxClients.Items;
            }
        }

        public int sendCommand(string command, bool all)
        {
            byte[] bytes = UTF8Encoding.UTF8.GetBytes(command);
            return sendCommand(bytes, all);
        }

        public int sendKey(int code)
        {
            return sendCommand("KEY " + code, false);
        }

        public int sendOpenApp(string pkgName)
        {
            if (checkBoxAutoUnlock.Checked)
            {
                return sendCommand("UNLOCK " + pkgName, true);
            }

            return sendCommand("OPEN " + pkgName, false);
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            if (mService.Running)
            {
                mService.stop();
            }
            else
            {
                mService.start();
                SetClipboardViewer();
            }
        }

        public override CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            return new NetworkImeClient(mService, conn);
        }

        public override void onTcpClientConnected(object sender, EventArgs e)
        {
            Console.WriteLine("onTcpClientConnected: sender = " + sender);
            checkedListBoxClients.Items.Add(sender, true);
        }

        public override void onTcpClientDisconnected(object sender, EventArgs e)
        {
            Console.WriteLine("onTcpClientDisconnected: sender = " + sender);
            checkedListBoxClients.Items.Remove(sender);
        }

        public override void onTcpClientUpdated(object sender, EventArgs e)
        {
            Console.WriteLine("onTcpClientUpdated: sender = " + sender);
            checkedListBoxClients.Invalidate();
        }

        private void addHistory(string text)
        {
            var items = comboBoxHistory.Items;
            items.Remove(text);
            items.Insert(0, text);
        }

        private void buttonSend_Click(object sender, EventArgs e)
        {
            string text = textBoxContent.Text;
            string command;

            switch (comboBoxSend.SelectedIndex)
            {
                case 1:
                    command = "REPLACE";
                    break;

                case 2:
                    command = "INSERT";
                    break;

                default:
                    command = "SEND";
                    break;
            }

            if (text != null && text.Length > 0)
            {
                addHistory(text);
                command += " " + text;
            }

            sendCommand(command, false);

            if (checkBoxClear.Checked)
            {
                textBoxContent.Clear();
            }
        }

        private void buttonClear_Click(object sender, EventArgs e)
        {
            sendCommand("CLEAR", true);
        }

        private void buttonVolume_Click(object sender, EventArgs e)
        {
            var dialog = new FormVolumeEditor();
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                sendCommand("VOLUME =" + dialog.getVolume(), false);
            }
        }

        private void buttonVolumeDown_Click(object sender, EventArgs e)
        {
            sendCommand("VOLUME -", false);
        }

        private void buttonVolumeUp_Click(object sender, EventArgs e)
        {
            sendCommand("VOLUME +", false);
        }

        private void buttonMute_Click(object sender, EventArgs e)
        {
            sendCommand("VOLUME x", false);
        }

        private void radioButtonSend_CheckedChanged(object sender, EventArgs e)
        {
            checkBoxClear.Checked = false;
            checkBoxEnterSend.Checked = true;
        }

        private void radioButtonReplace_CheckedChanged(object sender, EventArgs e)
        {
            checkBoxClear.Checked = false;
            checkBoxEnterSend.Checked = false;
        }

        private void radioButtonInsert_CheckedChanged(object sender, EventArgs e)
        {
            checkBoxClear.Checked = true;
            checkBoxEnterSend.Checked = false;
        }

        public override void onTcpServiceRunning(object sender, EventArgs e)
        {
            labelStatus.Text = "服务器正在运行";
        }

        public override void onTcpServiceStarted(object sender, EventArgs e)
        {
            Settings.Default.NetworkImePort = mService.Port;
            labelStatus.Text = "服务器已启动";
            buttonStart.Text = "停止";
        }

        public override void onTcpServiceStopped(object sender, EventArgs e)
        {
            Settings.Default.NetworkImeEnable = false;
            labelStatus.Text = "服务器已停止";
            buttonStart.Text = "启动";
        }

        public override void onTcpServiceWaiting(object sender, EventArgs e)
        {
            labelStatus.Text = "服务器正在等待";
        }

        public override void onTcpCommandReceived(object sender, EventArgs e)
        {
            var args = (e as CavanEventArgs<string[]>).Args;

            switch (args[0])
            {
                case "RESPONSE":
                    if (mFormSendCommand == null || mFormSendCommand.IsDisposed)
                    {
                        break;
                    }

                    mFormSendCommand.postResponse(args[1]);
                    break;
            }
        }

        private void FormNetworkIme_FormClosing(object sender, FormClosingEventArgs e)
        {
            Settings.Default.Save();

            e.Cancel = true;
            WindowState = FormWindowState.Minimized;
            ShowInTaskbar = false;
            Visible = false;
            SetClipboardViewer();
        }


        private void notifyIcon_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                WindowState = FormWindowState.Normal;
                ShowInTaskbar = true;
                Visible = true;
                Focus();
                SetClipboardViewer();
            }
        }

        private void backgroundWorkerRepeater_DoWork(object sender, DoWorkEventArgs e)
        {
            while (true)
            {
                if (backgroundWorkerRepeater.CancellationPending)
                {
                    break;
                }

                byte[] bytes = mRepeatSendBytes;
                if (bytes == null)
                {
                    break;
                }

                sendCommand(bytes, false);

                var index = comboBoxRepeat.SelectedIndex;
                if (index > 0)
                {
                    Thread.Sleep(index * 100);
                }
                else
                {
                    break;
                }
            }
        }

        private void backgroundWorkerRepeater_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            mRepeatSendBytes = null;
            comboBoxRepeat.SelectedIndex = 0;
        }

        private void checkBoxSelectAll_CheckedChanged(object sender, EventArgs e)
        {
            if (mBusyLock.acquire(checkBoxSelectAll))
            {
                int index = checkedListBoxClients.Items.Count;
                bool value = checkBoxSelectAll.Checked;

                while (index > 0)
                {
                    checkedListBoxClients.SetItemChecked(--index, value);
                }
            }
        }

        private void checkedListBoxClients_ItemCheck(object sender, ItemCheckEventArgs e)
        {
            if (!mBusyLock.acquire(checkedListBoxClients))
            {
                return;
            }

            if (e.NewValue == CheckState.Checked)
            {
                int index = checkedListBoxClients.Items.Count;
                bool checkedAll = true;

                while (index > 0)
                {
                    if (checkedListBoxClients.GetItemCheckState(--index) != CheckState.Checked && index != e.Index)
                    {
                        checkedAll = false;
                        break;
                    }
                }

                checkBoxSelectAll.Checked = checkedAll;
            }
            else
            {
                checkBoxSelectAll.Checked = false;
            }
        }

        private void buttonDelete_Click(object sender, EventArgs e)
        {
            sendKey(67);
        }

        private void buttonCopy_Click(object sender, EventArgs e)
        {
            sendCommand("COPY", false);
        }

        private void buttonPaste_Click(object sender, EventArgs e)
        {
            var text = Clipboard.GetText();
            if (text != null && text.Length > 0)
            {
                sendCommand("SEND " + text, false);
            }
        }

        private void buttonComplete_Click(object sender, EventArgs e)
        {
            sendCommand("COMMIT", false);
        }

        private void buttonBack_Click(object sender, EventArgs e)
        {
            sendCommand("BACK", false);
        }

        private void buttonOpen_Click(object sender, EventArgs e)
        {
            if (mFormOpenApp == null || mFormOpenApp.IsDisposed)
            {
                mFormOpenApp = new FormOpenApp(this);
            }

            mFormOpenApp.Show();
        }

        private void buttonSelect_Click(object sender, EventArgs e)
        {
            if (mFormSelect != null)
            {
                mFormSelect.Dispose();
            }

            mFormSelect = new FormSelect(this);
            mFormSelect.Show();
        }

        private void buttonIme_Click(object sender, EventArgs e)
        {
            sendCommand("IME", false);
        }

        private void buttonHome_Click(object sender, EventArgs e)
        {
            sendCommand("HOME", false);
        }

        private void timerKeepAlive_Tick(object sender, EventArgs e)
        {
            foreach (object item in checkedListBoxClients.Items)
            {
                NetworkImeClient client = item as NetworkImeClient;
                if (!client.sendPing())
                {
                    Console.WriteLine("disconnect keepalive");
                    client.disconnect();
                }
            }
        }

        private void buttonRequest_Click(object sender, EventArgs e)
        {
            if (mFormSender == null || mFormSender.IsDisposed)
            {
                mFormSender = new FormHttpSender();
            }

            mFormSender.Show();
        }

        private void toolStripMenuItemExit_Click(object sender, EventArgs e)
        {
            mService.stop();
            Dispose(true);
        }

        private void buttonRecents_Click(object sender, EventArgs e)
        {
            sendCommand("RECENTS", false);
        }

        private void buttonRefresh_Click(object sender, EventArgs e)
        {
            sendCommand("REFRESH", false);
        }

        private void buttonSignin_Click(object sender, EventArgs e)
        {
            sendCommand("SIGNIN", true);
        }

        private void buttonUnfollow_Click(object sender, EventArgs e)
        {
            sendCommand("UNFOLLOW", false);
        }

        private void buttonBuildPack_Click(object sender, EventArgs e)
        {
            if (mFormBuilder == null || mFormBuilder.IsDisposed)
            {
                mFormBuilder = new FormPackBuilder();
            }

            mFormBuilder.Show();
        }

        private void comboBoxHistory_TextChanged(object sender, EventArgs e)
        {
            var text = textBoxContent.Text;
            if (text != null && text.Length > 0)
            {
                addHistory(text);
            }

            textBoxContent.Text = comboBoxHistory.Text;
        }

        private void buttonWeixin_Click(object sender, EventArgs e)
        {
            sendOpenApp("com.tencent.mm");
        }

        private void buttonUnlock_Click(object sender, EventArgs e)
        {
            sendCommand("UNLOCK", false);
        }

        private void buttonCommand_Click(object sender, EventArgs e)
        {
            if (mFormSendCommand == null || mFormSendCommand.IsDisposed)
            {
                mFormSendCommand = new FormSendCommand(this);
            }

            mFormSendCommand.Show();
        }

        private void buttonAlipay_Click(object sender, EventArgs e)
        {
            if (mFormAlipay == null || mFormAlipay.IsDisposed)
            {
                mFormAlipay = new FormAlipay(this);
            }

            mFormAlipay.Show();

            var text = getClipboardText();
            mFormAlipay.setClipboardText(text);
        }

        private void buttonWeibo_Click(object sender, EventArgs e)
        {
            sendOpenApp("com.sina.weibo");
        }

        private void buttonTaobao_Click(object sender, EventArgs e)
        {
            sendOpenApp("com.taobao.taobao");
        }

        private void buttonTmall_Click(object sender, EventArgs e)
        {
            sendOpenApp("com.tmall.wireless");
        }

        private void buttonDesktop_Click(object sender, EventArgs e)
        {
            sendCommand("DESKTOP", false);
        }

        private void buttonShareFriends_Click(object sender, EventArgs e)
        {
            string command;

            if (checkBoxFakeShare.Checked)
            {
                command = "SHARE 4";
            }
            else
            {
                command = "SHARE 1";
            }

            sendCommand(command, true);
        }

        private void buttonShare_Click(object sender, EventArgs e)
        {
            string command;

            if (checkBoxFakeShare.Checked)
            {
                command = "SHARE 8";
            }
            else
            {
                command = "SHARE 2";
            }

            sendCommand(command, true);
        }

        private void buttonLock_Click(object sender, EventArgs e)
        {
            sendCommand("LOCK", true);
        }

        private void toolStripMenuItemTcpProxy_Click(object sender, EventArgs e)
        {
            if (mFormTcpProxy == null || mFormTcpProxy.IsDisposed)
            {
                mFormTcpProxy = new FormTcpProxyService();
            }

            mFormTcpProxy.Show();
        }

        private void toolStripMenuItemWebProxy_Click(object sender, EventArgs e)
        {
            if (mFormWebProxy == null || mFormWebProxy.IsDisposed)
            {
                mFormWebProxy = new FormWebProxyService();
            }

            mFormWebProxy.Show();
        }

        private void checkBoxFloatClock_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBoxFloatClock.Checked)
            {
                sendCommand("CLOCK 1", true);
            }
            else
            {
                sendCommand("CLOCK 0", true);
            }
        }

        private void contextMenuStrip_Opening(object sender, CancelEventArgs e)
        {
            toolStripMenuItemAutoRun.Checked = isAutoRunEnabled();
        }

        private void comboBoxRepeat_SelectedIndexChanged(object sender, EventArgs e)
        {

            if (comboBoxRepeat.SelectedIndex > 0)
            {
                string text = textBoxContent.Text;
                if (text != null && text.Length > 0)
                {
                    mRepeatSendBytes = UTF8Encoding.UTF8.GetBytes("SEND " + text);

                    if (!backgroundWorkerRepeater.IsBusy)
                    {
                        backgroundWorkerRepeater.RunWorkerAsync();
                    }
                }
            }
            else
            {
                mRepeatSendBytes = null;
                backgroundWorkerRepeater.CancelAsync();
            }
        }

        private void toolStripMenuItemUrlBuilder_Click(object sender, EventArgs e)
        {
            if (mFormUrlBuilder == null || mFormUrlBuilder.IsDisposed)
            {
                mFormUrlBuilder = new FormUrlBuilder();
            }

            mFormUrlBuilder.Show();
        }

        private void toolStripMenuItemAutoRun_DropDownOpening(object sender, EventArgs e)
        {
            toolStripMenuItemNetworkImeAuto.Checked = Settings.Default.NetworkImeEnable;
            toolStripMenuItemWebProxyAuto.Checked = Settings.Default.WebProxyEnable;
        }

        private void toolStripMenuItemNetworkImeAuto_CheckedChanged(object sender, EventArgs e)
        {
            Settings.Default.NetworkImeEnable = toolStripMenuItemNetworkImeAuto.Checked;
        }

        private void toolStripMenuItemWebProxyAuto_CheckedChanged(object sender, EventArgs e)
        {
            Settings.Default.WebProxyEnable = toolStripMenuItemWebProxyAuto.Checked;
        }

        private void toolStripMenuItemAutoRun_CheckedChanged(object sender, EventArgs e)
        {
            setAutoRunEnable(toolStripMenuItemAutoRun.Checked);
        }
    }

    public class NetworkImeClient : CavanTcpPacketClient
    {
        private int mKeepAlive;
        private string mUserName;

        internal delegate void TcpClientReceivedEventHandler(object sender, CavanEventArgs<string[]> e);

        public NetworkImeClient(CavanTcpService service, TcpClient client) : base(service, client)
        {
        }

        protected override void onDataPacketReceived(byte[] bytes, int length)
        {
            char[] chars = UTF8Encoding.UTF8.GetChars(bytes, 0, length);
            string command = new String(chars);

            // Console.WriteLine("onDataPacketReceived: " + command);

            string[] args = command.Split(new char[] { ' ' }, 2);
            args[0] = args[0].Trim().ToUpper();

            switch (args[0])
            {
                case "USER":
                    if (args.Length > 1)
                    {
                        string name = args[1].Trim();
                        if (name.Length > 0)
                        {
                            mUserName = name;
                            mService.onTcpClientUpdated(this);
                        }

                        Console.WriteLine("user = " + mUserName);
                    }
                    break;

                case "PING":
                    if (!sendPong())
                    {
                        disconnect();
                    }
                    break;

                case "PONG":
                    mKeepAlive = 0;
                    break;

                default:
                    mService.onTcpCommandReceived(this, args);
                    break;
            }
        }

        public bool sendPing()
        {
            if (mKeepAlive > 0)
            {
                if (mKeepAlive < 3)
                {
                    mKeepAlive++;
                    return true;
                }

                return false;
            }
            else
            {
                mKeepAlive = 1;
                return send("PING");
            }
        }

        public bool sendPong()
        {
            return send("PONG");
        }

        public override string ToString()
        {
            string text = base.ToString();

            if (mUserName != null)
            {
                return text + " - " + mUserName;
            }

            return text;
        }
    }
}
