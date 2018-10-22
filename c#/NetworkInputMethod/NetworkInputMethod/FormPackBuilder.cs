using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.IO;
using System.Net;
using System.Windows.Forms;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace NetworkInputMethod
{
    public partial class FormPackBuilder : Form
    {
        public const int IMG_INDEX_SERVER = 0;
        public const int IMG_INDEX_FILE = 1;
        public const int IMG_INDEX_ACCOUNT = 2;
        public const int IMG_INDEX_ITEM = 3;
        public const int IMG_INDEX_SELECT = 4;

        public static string CONFIG_FILE_PATH = Path.Combine(Application.LocalUserAppDataPath, "pack.json");

        public static string[] DEF_SERVERS = { "fuangcao.f3322.net:8021", "fuangcao.f3322.net:8022", "192.168.88.140:8021" };
        public static string[] DEF_FILES = { "/temp/weixin/game-250.txt", "/temp/weixin/game-47.txt", "/temp/weixin/game-140.txt" };

        public FormPackBuilder()
        {
            InitializeComponent();
            comboBoxReqCount.SelectedIndex = 0;
            load();

            if (treeView.Nodes.Count < 1)
            {
                for (int i = 0; i < DEF_SERVERS.Length; i++)
                {
                    var server = treeView.Nodes.Add(DEF_SERVERS[i]);
                    server.ImageIndex = IMG_INDEX_SERVER;

                    var file = server.Nodes.Add(DEF_FILES[i]);
                    file.ImageIndex = IMG_INDEX_FILE;
                }
            }

            treeView.ExpandAll();
        }

        public TreeNode createAccountNode(TreeNode parent)
        {
            var node = parent.Nodes.Add("账号");
            node.ImageIndex = IMG_INDEX_ACCOUNT;
            parent.Expand();
            return node;
        }

        public bool postClipboard(string text)
        {
            var account = treeView.SelectedNode;
            if (account == null || account.Level < 1)
            {
                MessageBox.Show("请选择一个账号！");
                return false;
            }

            text = text.Trim();
            if (!text.StartsWith("GET"))
            {
                return false;
            }

            if (account.Level < 2)
            {
                var node = createAccountNode(account);
                treeView.SelectedNode = node;
                account = node;
            }
            else
            {
                while (account.Level > 2)
                {
                    account = account.Parent;
                }

                foreach (TreeNode node in account.Nodes)
                {
                    if (text.Equals(node.Text))
                    {
                        return true;
                    }
                }

                var count = comboBoxReqCount.SelectedIndex;

                if (count > 0 && account.Nodes.Count >= count)
                {
                    var node = createAccountNode(account.Parent);
                    treeView.SelectedNode = node;
                    account = node;
                }
            }

            var child = account.Nodes.Add(text);
            child.ImageIndex = IMG_INDEX_ITEM;
            account.ExpandAll();

            return true;
        }

        public string build(TreeNode file)
        {
            StringBuilder builder = new StringBuilder();
            int appended = 0;

            foreach (TreeNode account in file.Nodes)
            {
                if (account.Nodes.Count <= 0)
                {
                    continue;
                }

                if (appended > 0)
                {
                    builder.AppendLine().AppendLine();
                }

                appended = 0;

                foreach (TreeNode item in account.Nodes)
                {
                    if (item != account.FirstNode)
                    {
                        builder.AppendLine();
                    }

                    builder.AppendLine(item.Text);
                    appended++;
                }
            }

            return builder.ToString();
        }

        public void save(JsonTextWriter writer)
        {
            writer.WriteStartArray();

            foreach (TreeNode server in treeView.Nodes)
            {
                writer.WriteStartObject();

                writer.WritePropertyName("url");
                writer.WriteValue(server.Text);

                writer.WritePropertyName("files");
                writer.WriteStartArray();

                foreach (TreeNode file in server.Nodes)
                {
                    writer.WriteStartObject();

                    writer.WritePropertyName("path");
                    writer.WriteValue(file.Text);

                    writer.WritePropertyName("items");
                    writer.WriteStartArray();

                    foreach (TreeNode account in file.Nodes)
                    {
                        writer.WriteStartArray();

                        foreach (TreeNode item in account.Nodes)
                        {
                            writer.WriteValue(item.Text);
                        }

                        writer.WriteEndArray();
                    }

                    writer.WriteEndArray();

                    writer.WriteEndObject();
                }

                writer.WriteEndArray();

                writer.WriteEndObject();
            }

            writer.WriteEndArray();
        }

        public bool save()
        {
            Console.WriteLine("save json: " + CONFIG_FILE_PATH);

            using (var writer = new StreamWriter(CONFIG_FILE_PATH))
            {
                try
                {
                    save(new JsonTextWriter(writer));
                    MessageBox.Show("保存成功！");
                    return true;
                }
                catch (Exception e)
                {
                    MessageBox.Show(e.ToString());
                }
            }

            return false;
        }

        public void export(string dirname)
        {
            foreach (TreeNode server in treeView.Nodes)
            {
                foreach (TreeNode file in server.Nodes)
                {
                    var path = Path.Combine(dirname, Path.GetFileName(file.Text));
                    var text = build(file);

                    if (text.Length > 0)
                    {
                        File.WriteAllText(path, text);
                    }
                    else
                    {
                        File.Delete(path);
                    }
                }
            }
        }

        public bool load()
        {
            try
            {
                return load(CONFIG_FILE_PATH);
            }
            catch (Exception)
            {
                return false;
            }
        }

        public bool load(string pathname)
        {
            var text = File.ReadAllText(pathname);
            if (text == null)
            {
                return false;
            }

            var array = JArray.Parse(text);
            if (array == null)
            {
                return false;
            }

            foreach (var token in array)
            {
                var serverUrl = token.Value<string>("url");
                if (serverUrl == null)
                {
                    continue;
                }

                var serverFiles = token.Value<JArray>("files");
                if (serverFiles == null)
                {
                    continue;
                }

                var serverNode = treeView.Nodes.Add(serverUrl);
                serverNode.ImageIndex = IMG_INDEX_SERVER;

                foreach (var file in serverFiles)
                {
                    var filePath = file.Value<string>("path");
                    if (filePath == null)
                    {
                        continue;
                    }

                    var fileItems = file.Value<JArray>("items");
                    if (fileItems == null)
                    {
                        continue;
                    }

                    var fileNode = serverNode.Nodes.Add(filePath);
                    fileNode.ImageIndex = IMG_INDEX_FILE;

                    foreach (var account in fileItems.Value<JArray>())
                    {
                        var accountNode = createAccountNode(fileNode);

                        foreach (var item in account)
                        {
                            var itemNode = accountNode.Nodes.Add(item.Value<string>());
                            itemNode.ImageIndex = IMG_INDEX_ITEM;
                        }
                    }
                }
            }

            return true;
        }

        public bool upload(string server, string pathname, string text)
        {
            if (string.IsNullOrEmpty(text))
            {
                return true;
            }

#if false
            string timestamp = DateTime.Now.Ticks.ToString("x");

            //根据uri创建HttpWebRequest对象
            HttpWebRequest req = (HttpWebRequest)WebRequest.Create(new Uri(url));
            req.Method = "POST";
            req.AllowWriteStreamBuffering = false; //对发送的数据不使用缓存
            req.Timeout = 10000;  //设置获得响应的超时时间（20秒）
            req.ContentType = "multipart/form-data; boundary=" + timestamp;

            //头信息
            string boundary = "--" + timestamp;
            string header = string.Format(boundary + "\r\nContent-Disposition: form-data; name=\"{0}\";filename=\"{1}\"\r\nContent-Type:application/octet-stream\r\n\r\n", "file", name);
            byte[] postHeaderBytes = Encoding.UTF8.GetBytes(header);
            byte[] postBodyBytes = Encoding.UTF8.GetBytes(text);

            //结束边界
            byte[] boundaryBytes = Encoding.ASCII.GetBytes("\r\n--" + timestamp + "--\r\n");

            long length = postBodyBytes.Length + postHeaderBytes.Length + boundaryBytes.Length;

            req.ContentLength = length;//请求内容长度

            try
            {
                Stream postStream = req.GetRequestStream();

                //发送请求头部消息
                postStream.Write(postHeaderBytes, 0, postHeaderBytes.Length);
                postStream.Write(postBodyBytes, 0, postBodyBytes.Length);

                //添加尾部边界
                postStream.Write(boundaryBytes, 0, boundaryBytes.Length);
                postStream.Close();

                //获取服务器端的响应
                using (HttpWebResponse response = (HttpWebResponse)req.GetResponse())
                {
                    Stream receiveStream = response.GetResponseStream();
                    StreamReader readStream = new StreamReader(receiveStream, Encoding.UTF8);
                    string returnValue = readStream.ReadToEnd();
                    MessageBox.Show(returnValue);
                    response.Close();
                    readStream.Close();
                }

                return true;
            }
            catch (Exception err)
            {
                MessageBox.Show(url + "\r\n" + err);
            }
#else
            var url = "http://" + server + pathname;
            var client = new WebClient();

            try
            {
                var response = client.UploadString(url, text);

                if (response != null)
                {
                    MessageBox.Show(url + "\r\n" + response, "上传成功");
                    return true;
                }
            }
            catch (Exception err)
            {
                MessageBox.Show(url + "\r\n" + err, "上传失败");
            }
#endif

            return false;
        }

        public StringBuilder upload(StringBuilder builder, TreeNode server, TreeNode file)
        {
            var text = build(file);

            builder.Append(server.Text).Append('@').Append(file.Text).Append(": ");

            if (upload(server.Text, file.Text, text))
            {
                builder.Append("成功");
            }
            else
            {
                builder.Append("失败");
            }

            builder.AppendLine();

            return builder;
        }

        private void buttonFile_Click(object sender, EventArgs e)
        {
            var server = treeView.SelectedNode;
            if (server == null)
            {
                MessageBox.Show("请选择一个服务器！");
                return;
            }

            while (server.Level > 0)
            {
                server = server.Parent;
            }

            var form = new FormTextEditor("文件路径", null);
            if (form.ShowDialog() == DialogResult.OK)
            {
                var node = server.Nodes.Add(form.Value);
                node.ImageIndex = IMG_INDEX_FILE;
                server.Expand();
            }
        }

        private void buttonAccount_Click(object sender, EventArgs e)
        {
            var file = treeView.SelectedNode;
            if (file == null || file.Level < 1)
            {
                MessageBox.Show("请选择一个文件！");
                return;
            }

            while (file.Level > 1)
            {
                file = file.Parent;
            }

            createAccountNode(file);
        }

        private void buttonSave_Click(object sender, EventArgs e)
        {
            save();
        }

        private void buttonServer_Click(object sender, EventArgs e)
        {
            var form = new FormTextEditor("服务器Url", null);

            if (form.ShowDialog() == DialogResult.OK)
            {
                var node = treeView.Nodes.Add(form.Value);
                node.ImageIndex = IMG_INDEX_SERVER;
            }
        }

        private void buttonExport_Click(object sender, EventArgs e)
        {
            if (folderBrowserDialog.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    export(folderBrowserDialog.SelectedPath);
                    MessageBox.Show("导出成功！");
                }
                catch (Exception err)
                {
                    MessageBox.Show(err.ToString());
                }

            }
        }

        private void buttonDelete_Click(object sender, EventArgs e)
        {
            var node = treeView.SelectedNode;
            if (node != null)
            {
                node.Remove();
            }
        }

        private void buttonClear_Click(object sender, EventArgs e)
        {
            if (MessageBox.Show("确定要清空吗？", "确认清空", MessageBoxButtons.YesNo) == DialogResult.Yes)
            {
                foreach (TreeNode server in treeView.Nodes)
                {
                    foreach (TreeNode file in server.Nodes)
                    {
                        file.Nodes.Clear();
                    }
                }
            }
        }

        private void contextMenuStrip_Opening(object sender, CancelEventArgs e)
        {
            var node = treeView.SelectedNode;
            int level;

            if (node == null)
            {
                deleteToolStripMenuItem.Enabled = false;
                uploadToolStripMenuItem.Enabled = false;
                level = -1;
            }
            else
            {
                deleteToolStripMenuItem.Enabled = true;
                uploadToolStripMenuItem.Enabled = true;
                level = node.Level;
            }

            addFileToolStripMenuItem.Enabled = (level == 0);
            addAccountToolStripMenuItem.Enabled = (level == 1);
        }

        private void buttonUpload_Click(object sender, EventArgs e)
        {
            StringBuilder builder = new StringBuilder();

            foreach (TreeNode server in treeView.Nodes)
            {
                foreach (TreeNode file in server.Nodes)
                {
                    upload(builder, server, file);
                }
            }

            MessageBox.Show(builder.ToString());
        }

        private void uploadToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode node = treeView.SelectedNode;
            if (node != null)
            {
                var builder = new StringBuilder();

                if (node.Level > 0)
                {
                    while (node.Level > 1)
                    {
                        node = node.Parent;
                    }

                    upload(builder, node.Parent, node);
                }
                else
                {
                    foreach (TreeNode file in node.Nodes)
                    {
                        upload(builder, node, file);
                    }
                }

                MessageBox.Show(builder.ToString());
            }
        }
    }
}
