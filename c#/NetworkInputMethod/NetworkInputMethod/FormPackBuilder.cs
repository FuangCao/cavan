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
        static string CONFIG_FILE = Path.Combine(Application.LocalUserAppDataPath, "pack.json");

        public FormPackBuilder()
        {
            InitializeComponent();
        }

        public TreeNode createAccountNode()
        {
            var node = new TreeNode();
            node.ImageIndex = 2;
            node.Text = "账号";
            return node;
        }

        public void postClipboard(string text)
        {
            var accouunt = treeView.SelectedNode;
            if (accouunt == null || accouunt.Level < 2)
            {
                MessageBox.Show("请选择一个账号！");
                return;
            }

            while (accouunt.Level > 2)
            {
                accouunt = accouunt.Parent;
            }

            var child = accouunt.Nodes.Add(text);
            child.ImageIndex = 2;
            accouunt.ExpandAll();
        }

        public string build()
        {
            return null;
        }

        public void parse()
        {
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
                node.ImageIndex = 1;
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

            var node = file.Nodes.Add("账号");
            node.ImageIndex = 2;
            file.Expand();
        }

        private void buttonSave_Click(object sender, EventArgs e)
        {
            Console.WriteLine("pathname = " + CONFIG_FILE);

            using (var writer = new StreamWriter(CONFIG_FILE))
            {
                var jwriter = new JsonTextWriter(writer);

                jwriter.WriteStartArray();

                foreach (TreeNode server in treeView.Nodes)
                {
                    jwriter.WriteStartObject();

                    jwriter.WritePropertyName("url");
                    jwriter.WriteValue(server.Text);

                    jwriter.WritePropertyName("files");
                    jwriter.WriteStartArray();

                    foreach (TreeNode file in server.Nodes)
                    {
                        jwriter.WriteStartObject();

                        jwriter.WritePropertyName("path");
                        jwriter.WriteValue(file.Text);

                        jwriter.WritePropertyName("items");
                        jwriter.WriteStartArray();

                        foreach (TreeNode account in file.Nodes)
                        {
                            jwriter.WriteStartArray();

                            foreach (TreeNode item in account.Nodes)
                            {
                                jwriter.WriteValue(item.Text);
                            }

                            jwriter.WriteEndArray();
                        }

                        jwriter.WriteEndArray();

                        jwriter.WriteEndObject();
                    }

                    jwriter.WriteEndArray();

                    jwriter.WriteEndObject();
                }

                jwriter.WriteEndArray();
            }
        }

        private void buttonServer_Click(object sender, EventArgs e)
        {
            var form = new FormTextEditor("服务器Url", null);

            if (form.ShowDialog() == DialogResult.OK)
            {
                var node = treeView.Nodes.Add(form.Value);
                node.ImageIndex = 0;
            }
        }
    }
}
