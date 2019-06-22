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
using System.Text.RegularExpressions;
using System.Collections;
using System.IO;

namespace NetworkInputMethod
{
    public partial class FormUrlBuilder : FormTcpService
    {
        private CavanTcpService mService;
        private byte[] mWebContent;

        public FormUrlBuilder()
        {
            InitializeComponent();
            mService = new CavanTcpService(this);
        }

        public byte[] WebContent
        {
            get
            {
                return mWebContent;
            }
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
            return new UrlBuilderClient(this, mService, conn);
        }

        public bool postClipboard(string text)
        {
            if (!text.StartsWith("http"))
            {
                return false;
            }

            var question = text.IndexOf('?');

            if (question > 0)
            {
                var props = text.Substring(question + 1).Split('&');
                var count = 0;

                foreach (var prop in props)
                {
                    var equal = prop.IndexOf('=');

                    if (equal > 0)
                    {
                        var name = prop.Substring(0, equal);

                        Console.WriteLine("name = " + name);

                        if (name.Equals("key") || name.Equals("uin") || name.Equals("pass_ticket"))
                        {
                            Console.WriteLine("skip: " + prop);
                            continue;
                        }
                    }

                    props[count++] = prop;
                }

                if (count < props.Length)
                {
                    var builder = new StringBuilder();

                    builder.Append(text.Substring(0, question)).Append('?');

                    for (int i = 0; i < count; i++)
                    {
                        if (i > 0)
                        {
                            builder.Append('&');
                        }

                        builder.Append(props[i]);
                    }

                    text = builder.ToString();
                }
            }

            var items = listViewUrls.Items;
            if (items.ContainsKey(text))
            {
                return true;
            }

            var item = items.Add(text, items.Count.ToString(), 0);
            item.SubItems.Add("链接" + items.Count);
            item.SubItems.Add(text);

            return true;
        }

        private void buttonGenerate_Click(object sender, EventArgs e)
        {
            var builder = new StringBuilder();

            builder.AppendLine("<html>");
            builder.AppendLine("\t<head>");
            builder.AppendLine("\t\t<meta http-equiv=\"content -type\" content=\"text/html;charset=utf-8\" />");
            builder.AppendLine("\t\t<meta name=\"viewport\" content=\"initial-scale=1\" />");
            builder.AppendLine("\t\t<title>链接生成器</title>");
            builder.AppendLine("\t</head>");
            builder.AppendLine("\t<body>");

            foreach (ListViewItem item in listViewUrls.Items)
            {
                builder.AppendLine("\t\t<h3><a href=\"" + item.SubItems[2].Text + "\">" + item.SubItems[1].Text + "</a></h3>");
            }

            builder.AppendLine("\t</body>");
            builder.AppendLine("</html>");

            mWebContent = Encoding.UTF8.GetBytes(builder.ToString());

            builder = new StringBuilder();

            foreach (ListViewItem item in listViewUrls.Items)
            {
                if (builder.Length > 0)
                {
                    builder.AppendLine();
                }

                builder.Append(item.Index + 1).Append(". ").AppendLine(item.SubItems[2].Text);
            }

            Clipboard.SetText(builder.ToString());
        }

        private void buttonClear_Click(object sender, EventArgs e)
        {
            listViewUrls.Items.Clear();
        }

        private void toolStripMenuItemCopy_Click(object sender, EventArgs e)
        {
            var builder = new StringBuilder();

            foreach (ListViewItem item in listViewUrls.SelectedItems)
            {
                if (builder.Length > 0)
                {
                    builder.AppendLine();
                }

                builder.Append(item.Index).Append(". ").AppendLine(item.SubItems[2].Text);
            }

            Clipboard.SetText(builder.ToString());
        }
    }

    public class UrlBuilderClient : CavanTcpClient
    {
        private FormUrlBuilder mForm;

        public UrlBuilderClient(FormUrlBuilder form, CavanTcpService service, TcpClient client) : base(service, client)
        {
            mForm = form;
        }

        public override void mainLoop()
        {
            var stream = mClient.GetStream();
            var req = new CavanHttpRequest();

            if (!req.read(stream, null))
            {
                return;
            }

            var builder = new StringBuilder();

            var writer = new StreamWriter(stream);
            writer.WriteLine("HTTP/1.1 200 Gatewaying");

            var content = mForm.WebContent;
            if (content == null)
            {
                content = Encoding.UTF8.GetBytes("没有可用的链接");
                writer.WriteLine("Content-Type: text/plain");
            }
            else
            {
                writer.WriteLine("Content-Type: text/html");
            }

            writer.WriteLine("Content-Length: " + content.Length);

            writer.WriteLine();
            writer.Flush();
            stream.Write(content, 0, content.Length);
        }
    }
}
