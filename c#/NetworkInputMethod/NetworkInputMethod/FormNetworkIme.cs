using System;
using System.ComponentModel;
using System.Text;
using System.Windows.Forms;
using System.Net.Sockets;
using System.Threading;
using System.Runtime.InteropServices;
using System.Collections;

namespace NetworkInputMethod
{
    public partial class FormNetworkIme : Form
    {
        private const int WM_DRAWCLIPBOARD = 0x308;
        private const int WM_CHANGECBCHAIN = 0x30D;

        //API declarations...
        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        static public extern IntPtr SetClipboardViewer(IntPtr hWndNewViewer);
        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        static public extern bool ChangeClipboardChain(IntPtr HWnd, IntPtr HWndNext);
        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        public static extern int SendMessage(IntPtr hWnd, int msg, IntPtr wParam, IntPtr lParam);

        private byte[] mRepeatSendBytes;
        private NetworkImeService mService;
        private IntPtr mNextClipboardViewer;
        private CavanBusyLock mBusyLock = new CavanBusyLock();

        delegate void SimpleDelegate(Object obj);

        public FormNetworkIme()
        {
            InitializeComponent();
            mService = new NetworkImeService(this);
            mNextClipboardViewer = SetClipboardViewer(Handle);
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

        private void onClipboardChanged()
        {
            string text = Clipboard.GetText();
            Console.WriteLine("onClipboardChanged: " + text);

            if (checkBoxShareClipboard.Checked && text != null && text.Length > 0)
            {
                string command = "CLIPBOARD " + text;
                sendCommand(command, true);
            }
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
            return sendCommand("OPEN " + pkgName, false);
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            if (mService.Enabled)
            {
                mService.stop(false);
                buttonStart.Text = "启动";
            }
            else
            {
                mService.start();
                buttonStart.Text = "停止";
            }
        }

        public void onTcpClientConnected(object sender, EventArgs e)
        {
            Console.WriteLine("onTcpClientConnected: sender = " + sender);
            checkedListBoxClients.Items.Add(sender, true);
        }

        public void onTcpClientDisconnected(object sender, EventArgs e)
        {
            Console.WriteLine("onTcpClientDisconnected: sender = " + sender);
            checkedListBoxClients.Items.Remove(sender);
        }

        public void onTcpClientUpdated(object sender, EventArgs e)
        {
            Console.WriteLine("onTcpClientUpdated: sender = " + sender);
            checkedListBoxClients.Invalidate();
        }

        private void buttonSend_Click(object sender, EventArgs e)
        {
            string text = textBoxContent.Text;
            string command;

            if (radioButtonInsert.Checked)
            {
                command = "INSERT";
            }
            else if (radioButtonReplace.Checked)
            {
                command = "REPLACE";
            }
            else
            {
                command = "SEND";
            }

            if (text != null && text.Length > 0)
            {
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
            textBoxContent.Clear();
            sendCommand("REPLACE", false);
        }

        private void buttonUp_Click(object sender, EventArgs e)
        {
            sendKey(19);
        }

        private void buttonDown_Click(object sender, EventArgs e)
        {
            sendKey(20);
        }

        private void buttonLeft_Click(object sender, EventArgs e)
        {
            sendKey(21);
        }

        private void buttonRight_Click(object sender, EventArgs e)
        {
            sendKey(22);
        }

        private void buttonEnter_Click(object sender, EventArgs e)
        {
            sendKey(66);
        }

        private void buttonVolumeDown_Click(object sender, EventArgs e)
        {
            sendKey(25);
        }

        private void buttonVolumeUp_Click(object sender, EventArgs e)
        {
            sendKey(24);
        }

        private void buttonBack_Click(object sender, EventArgs e)
        {
            sendKey(4);
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

        internal void onTcpServiceRunning(object sender, EventArgs e)
        {
            labelStatus.Text = "服务器正在运行";
        }

        internal void onTcpServiceStarted(object sender, EventArgs e)
        {
            labelStatus.Text = "服务器已启动";
        }

        internal void onTcpServiceStopped(object sender, EventArgs e)
        {
            labelStatus.Text = "服务器已停止";
        }

        internal void onTcpServiceWaiting(object sender, EventArgs e)
        {
            labelStatus.Text = "服务器正在等待";
        }

        private void checkBoxRepeat_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBoxRepeat.Checked)
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

        private void FormNetworkIme_FormClosing(object sender, FormClosingEventArgs e)
        {
            mService.stop(true);
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
                Thread.Sleep(500);
            }
        }

        private void backgroundWorkerRepeater_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            mRepeatSendBytes = null;
            checkBoxRepeat.Checked = false;
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

        private void buttonAlipay_Click(object sender, EventArgs e)
        {
            sendOpenApp("com.eg.android.AlipayGphone");
        }

        private void buttonCopy_Click(object sender, EventArgs e)
        {
            sendCommand("COPY", false);
        }

        private void buttonPaste_Click(object sender, EventArgs e)
        {
            sendCommand("PASTE", false);
        }

        private void buttonSelectAll_Click(object sender, EventArgs e)
        {
            sendCommand("SELECT_ALL", false);
        }

        private void buttonComplete_Click(object sender, EventArgs e)
        {
            sendCommand("COMMIT", false);
        }

        private void buttonOpen_Click(object sender, EventArgs e)
        {
            FormOpenApp form = new FormOpenApp(this);
            form.Show();
        }
    }

    public class NetworkImeClient : CavanTcpPacketClient
    {
        private string mUserName;
        private NetworkImeService mService;

        public NetworkImeClient(NetworkImeService service, TcpClient client) : base(client)
        {
            mService = service;
        }

        protected override void onDataPacketReceived(byte[] bytes, int length)
        {
            char[] chars = UTF8Encoding.ASCII.GetChars(bytes, 0, length);
            string command = new String(chars);
            Console.WriteLine("onDataPacketReceived: " + command);
            string[] args = command.Split(new char[] { ' ' }, 2);
            Console.WriteLine("command = " + args[0]);

            switch (args[0])
            {
                case "USER":
                    if (args.Length > 1)
                    {
                        string name = args[1].Trim();
                        if (name.Length > 0)
                        {
                            mUserName = name;
                            FormNetworkIme form = mService.Form;
                            EventHandler handler = new EventHandler(form.onTcpClientUpdated);
                            form.Invoke(handler, this, null);
                        }

                        Console.WriteLine("user = " + mUserName);
                    }
                    break;
            }
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

    public class NetworkImeService : CavanTcpService
    {
        private FormNetworkIme mForm;

        public NetworkImeService(FormNetworkIme form)
        {
            mForm = form;
        }

        public FormNetworkIme Form
        {
            get
            {
                return mForm;
            }
        }

        private object Invoke(Delegate method, params object[] args)
        {
            try
            {
                return mForm.Invoke(method, args);
            }
            catch (Exception)
            {
                return null;
            }
        }

        protected override void onTcpServiceRunning()
        {
            EventHandler handler = new EventHandler(mForm.onTcpServiceRunning);
            Invoke(handler, this);
        }

        protected override void onTcpServiceStarted()
        {
            EventHandler handler = new EventHandler(mForm.onTcpServiceStarted);
            Invoke(handler, this);
        }

        protected override void onTcpServiceStopped()
        {
            EventHandler handler = new EventHandler(mForm.onTcpServiceStopped);
            Invoke(handler, this);
        }

        protected override void onTcpServiceWaiting()
        {
            EventHandler handler = new EventHandler(mForm.onTcpServiceWaiting);
            Invoke(handler, this);
        }

        protected override CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            return new NetworkImeClient(this, conn);
        }

        protected override void onTcpClientConnected(CavanTcpClient client)
        {
            EventHandler handler = new EventHandler(mForm.onTcpClientConnected);
            Invoke(handler, client, null);
        }

        protected override void onTcpClientDisconnected(CavanTcpClient client)
        {
            EventHandler handler = new EventHandler(mForm.onTcpClientDisconnected);
            Invoke(handler, client, null);
        }
    }
}
