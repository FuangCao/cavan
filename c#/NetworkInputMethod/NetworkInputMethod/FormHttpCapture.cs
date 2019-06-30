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

namespace NetworkInputMethod
{
    public partial class FormHttpCapture : FormTcpService
    {
        private CavanTcpService mService;

        public FormHttpCapture()
        {
            mService = new CavanTcpService(this);
            InitializeComponent();
            textBoxPort.Text = Settings.Default.HttpCapturePort.ToString();
        }

        private void buttonSwitch_Click(object sender, EventArgs e)
        {
            mService.toggle(textBoxPort.Text);
        }

        public override void onTcpServiceStarted(object sender, EventArgs e)
        {
            buttonSwitch.Text = "停止";
        }

        public override void onTcpServiceRunning(object sender, EventArgs e)
        {
            Settings.Default.HttpCapturePort = mService.Port;
        }

        public override void onTcpServiceStopped(object sender, EventArgs e)
        {
            buttonSwitch.Text = "启动";
        }

        public override CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            return new HttpCaptureClient(conn);
        }

        private void FormHttpCapture_Load(object sender, EventArgs e)
        {
            if (Settings.Default.HttpCaptureEnable)
            {
                buttonSwitch.PerformClick();
            }
        }
    }

    public class HttpCaptureClient : CavanTcpClient
    {
        public HttpCaptureClient(TcpClient client) : base(client)
        {
        }

        public override bool mainLoop()
        {
            var req = new CavanHttpRequest();
            if (!req.read(getStream(), null))
            {
                return false;
            }

            var url = req.Url;
            Console.WriteLine("url = " + url);

            return false;
        }
    }
}
