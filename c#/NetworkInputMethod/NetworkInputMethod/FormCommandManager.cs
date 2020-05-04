using NetworkInputMethod.Properties;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace NetworkInputMethod
{
    public partial class FormCommandManager : Form
    {
        public FormCommandManager()
        {
            InitializeComponent();
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            LoadListView();
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            Visible = false;
            e.Cancel = true;
        }

        public ListView ListViewCommands { get => listViewCommands; }

        public bool UseShellExecute { get => checkBoxUseShell.Checked; }

        public void SaveListView()
        {
            var commands = new StringCollection();

            foreach (ListViewItem item in listViewCommands.Items)
            {
                var thread = item.Tag as DaemonThread;
                commands.Add(thread.Path + "|" + thread.Args + "|" + thread.Enabled);
            }

            Settings.Default.DaemonCommands = commands;
            Settings.Default.Save();
        }

        public void LoadListView()
        {
            var commands = Settings.Default.DaemonCommands;
            if (commands != null)
            {
                foreach (var command in commands)
                {
                    Console.WriteLine(command);

                    var args = command.Split('|');
                    if (args.Length < 2)
                    {
                        continue;
                    }

                    var thread = new DaemonThread(this, args[0], args[1]);

                    if (args.Length > 2 && bool.Parse(args[2]))
                    {
                        thread.Start();
                    }
                }
            }
        }

        public void StopDaemons(bool clear)
        {
            var names = new HashSet<string>();

            foreach (ListViewItem item in listViewCommands.Items)
            {
                var thread = item.Tag as DaemonThread;
                var name = thread.Stop(true);

                if (name != null)
                {
                    names.Add(name);
                }
                else
                {
                    names.Add(Path.GetFileNameWithoutExtension(thread.Path));
                }
            }

            if (clear)
            {
                try
                {
                    foreach (var name in names)
                    {
                        foreach (var process in Process.GetProcessesByName(name))
                        {
                            process.Kill();
                            process.WaitForExit(2000);
                        }
                    }
                }
                catch (Exception err)
                {
                    Console.WriteLine(err);
                }
            }
        }

        private void toolStripMenuItemStart_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listViewCommands.SelectedItems)
            {
                var thread = item.Tag as DaemonThread;
                thread.Start();
            }

            SaveListView();
        }

        private void toolStripMenuItemStop_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listViewCommands.SelectedItems)
            {
                var thread = item.Tag as DaemonThread;
                thread.Stop(true);
            }

            SaveListView();
        }

        private void toolStripMenuItemEdit_Click(object sender, EventArgs e)
        {
            var items = listViewCommands.SelectedItems;
            if (items != null && items.Count > 0)
            {
                var thread = items[0].Tag as DaemonThread;

                var form = new FormCommandEditor()
                {
                    Command = thread.Path,
                    Args = thread.Args,
                };

                if (form.ShowDialog() == DialogResult.OK)
                {
                    thread.Path = form.Command;
                    thread.Args = form.Args;
                    thread.Stop(false);
                }

                SaveListView();
            }
        }

        private void toolStripMenuItemRemove_Click(object sender, EventArgs e)
        {
            var items = new List<ListViewItem>();

            foreach (ListViewItem item in listViewCommands.SelectedItems)
            {
                var thread = item.Tag as DaemonThread;
                thread.Stop(true);
                items.Add(item);
            }

            foreach (var item in items)
            {
                listViewCommands.Items.Remove(item);
            }

            SaveListView();
        }

        private void toolStripMenuItemAdd_Click(object sender, EventArgs e)
        {
            var form = new FormCommandEditor();

            if (form.ShowDialog() == DialogResult.OK)
            {
                new DaemonThread(this, form.Command, form.Args);
                SaveListView();
            }
        }

        private void toolStripMenuItemRestart_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listViewCommands.SelectedItems)
            {
                var thread = item.Tag as DaemonThread;
                thread.Stop(false);
            }
        }

        private void contextMenuStripCommands_Opening(object sender, CancelEventArgs e)
        {
            var items = listViewCommands.SelectedItems;
            toolStripMenuItemEdit.Enabled = (items.Count == 1);
            toolStripMenuItemRemove.Enabled = toolStripMenuItemStart.Enabled = toolStripMenuItemStop.Enabled = toolStripMenuItemRestart.Enabled = (items.Count > 0);
        }

        private void buttonStart_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listViewCommands.Items)
            {
                var thread = item.Tag as DaemonThread;
                thread.Start();
            }

            SaveListView();
        }

        private void buttonStop_Click(object sender, EventArgs e)
        {
            StopDaemons(false);
            SaveListView();
        }

        private void buttonRestart_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in listViewCommands.Items)
            {
                var thread = item.Tag as DaemonThread;
                thread.Stop(false);
            }
        }

        private void buttonClear_Click(object sender, EventArgs e)
        {
            StopDaemons(true);
            SaveListView();
        }
    }

    public class DaemonThread
    {
        public delegate void DaemonStateUpdateHandler(string state, Color color);

        private FormCommandManager mManager;
        private ListViewItem mItem;
        private Thread mThread;

        public string State { get => mItem.SubItems[1].Text; set => mItem.SubItems[1].Text = value; }

        public string Path { get => mItem.SubItems[2].Text; set => mItem.SubItems[2].Text = value; }

        public string Args { get => mItem.SubItems[3].Text; set => mItem.SubItems[3].Text = value; }

        public Process Process { get; private set; }

        public bool Enabled { get; set; }

        public DaemonThread(FormCommandManager manager, string path, string args)
        {
            mManager = manager;

            var view = manager.ListViewCommands;

            var items = view.Items;
            var item = items.Add(items.Count.ToString());
            item.SubItems.Add("未启动");
            item.SubItems.Add(path);
            item.SubItems.Add(args);
            item.Tag = this;
            mItem = item;
        }

        public void Start()
        {
            lock (this)
            {
                Enabled = true;

                if (mThread == null)
                {
                    mThread = new Thread(new ThreadStart(DaemonLoop)) { IsBackground = true };
                    mThread.Start();
                }

                Monitor.Pulse(this);
            }
        }

        public string Stop(bool exit)
        {
            string name = null;

            lock (this)
            {
                if (exit)
                {
                    Enabled = false;
                    mThread = null;
                }

                var process = Process;
                if (process != null)
                {
                    Process = null;
                    name = process.ProcessName;

                    process.Kill();
                    process.WaitForExit(2000);
                }

                Monitor.Pulse(this);
            }

            return name;
        }

        private void DaemonLoop()
        {
            var thread = mThread;

            while (Enabled && thread == mThread)
            {
                try
                {
                    PerformStateUpdate("正在启动", Color.Blue);
                    var info = new ProcessStartInfo()
                    {
                        FileName = Path,
                        Arguments = Args,
                    };

                    if (mManager.UseShellExecute)
                    {
                        info.CreateNoWindow = false;
                        info.UseShellExecute = true;
                    }
                    else
                    {
                        info.CreateNoWindow = true;
                        info.UseShellExecute = false;
                    }

                    var process = Process.Start(info);
                    Process = process;

                    PerformStateUpdate("正在运行", Color.Green);
                    process.WaitForExit();
                }
                catch (Exception err)
                {
                    Console.WriteLine(err);

                    if (!File.Exists(Path))
                    {
                        break;
                    }
                }
                finally
                {
                    Process = null;
                }

                if (Enabled)
                {
                    PerformStateUpdate("正在等待", Color.Red);

                    lock (this)
                    {
                        Monitor.Wait(this, 5000);
                    }
                }
            }

            lock (this)
            {
                if (mThread == thread)
                {
                    mThread = null;
                }
            }

            PerformStateUpdate("已停止", Color.Black);
        }

        private void PerformStateUpdate(string state, Color color)
        {
            mManager.Invoke(new DaemonStateUpdateHandler(OnStateUpdated), state, color);
        }

        private void OnStateUpdated(string state, Color color)
        {
            mItem.ForeColor = Color.White;
            mItem.BackColor = color;
            State = state;
        }
    }
}
