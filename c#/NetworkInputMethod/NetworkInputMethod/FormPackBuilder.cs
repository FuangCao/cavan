using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.IO;
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

        public void postClipboard(string text)
        {
            var account = treeView.SelectedNode;
            if (account == null || account.Level < 1)
            {
                MessageBox.Show("请选择一个账号！");
                return;
            }

            text = text.Trim();

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
                        return;
                    }
                }
            }

            var child = account.Nodes.Add(text);
            child.ImageIndex = IMG_INDEX_ITEM;
            account.ExpandAll();
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

        public void parse()
        {
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
            foreach (TreeNode server in treeView.Nodes)
            {
                foreach (TreeNode file in server.Nodes)
                {
                    file.Nodes.Clear();
                }
            }
        }
    }
}
