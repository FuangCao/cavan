using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;

namespace NetworkInputMethod
{
    public partial class FormUrlBuilder : FormTcpService
    {
        private CavanTcpService mService;

        public FormUrlBuilder()
        {
            InitializeComponent();
            mService = new CavanTcpService(this);
        }

        private void buttonSwitch_Click(object sender, EventArgs e)
        {
            mService.Port = Convert.ToInt32(textBoxPort.Text);

            if (mService.Running)
            {
                mService.stop();
            }
            else
            {
                mService.start();
            }
        }

        public override void onTcpServiceStarted(object sender, EventArgs e)
        {
            buttonSwitch.Text = "停止";
            textBoxPort.Enabled = false;
        }

        public override void onTcpServiceStopped(object sender, EventArgs e)
        {
            buttonSwitch.Text = "启动";
            textBoxPort.Enabled = true;
        }

        public override CavanTcpClient onTcpClientAccepted(TcpClient conn)
        {
            return new UrlBuilderClient(mService, conn);
        }
    }

    public class UrlBuilderClient : CavanTcpClient
    {
        public UrlBuilderClient(CavanTcpService service, TcpClient client) : base(service, client)
        {
        }

        public override void mainLoop()
        {
            var req = new CavanHttpRequest();

            if (!req.read(mClient.GetStream(), null))
            {
                return;
            }
        }
    }
}
