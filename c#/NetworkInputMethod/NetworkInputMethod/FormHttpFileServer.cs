using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Windows.Forms;
using NetworkInputMethod.Properties;
using System.IO;

namespace NetworkInputMethod
{
    public partial class FormHttpFileServer : FormTcpService
    {
        private CavanTcpService mService;

        public FormHttpFileServer()
        {
            mService = new CavanTcpService(this);
            InitializeComponent();
            textBoxPort.Text = Settings.Default.FileServerPort.ToString();
        }

        private void buttonSwitch_Click(object sender, EventArgs e)
        {
            mService.toggle(textBoxPort.Text);
        }

        public override void onTcpServiceStarted(object sender, EventArgs e)
        {
            textBoxPort.Enabled = false;
            buttonSwitch.Text = "停止";
        }

        public override void onTcpServiceStopped(object sender, EventArgs e)
        {
            textBoxPort.Enabled = true;
            buttonSwitch.Text = "启动";
        }

        public override void onTcpServiceRunning(object sender, EventArgs e)
        {
            Settings.Default.FileServerPort = mService.Port;
        }

        private void FormHttpFileServer_Load(object sender, EventArgs e)
        {
            if (Settings.Default.FileServerEnable)
            {
                buttonSwitch.PerformClick();
            }
        }

        public override CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            return new HttpFileLink(conn);
        }
    }

    public class HttpFileLink : CavanTcpClient
    {
        public HttpFileLink(TcpClient client) : base(client)
        {
        }

        public override bool mainLoop()
        {
            var stream = mClient.GetStream();
            var reader = new CavanHttpReader(stream);
            var lines = reader.ReadLines();

            Console.WriteLine("lines = " + lines);

            if (lines != null)
            {
                foreach (var line in lines)
                {
                    Console.WriteLine(line);
                }
            }

            foreach (var drive in DriveInfo.GetDrives())
            {
                var builder = new StringBuilder();
                builder.AppendLine("Name = " + drive.Name);
                builder.AppendLine("RootDirectory = " + drive.RootDirectory);
                builder.AppendLine("DriveType = " + drive.DriveType);
                Console.WriteLine(builder.ToString());
            }

            return false;
        }
    }
}
