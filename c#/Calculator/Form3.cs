using System;
using System.Windows.Forms;
using System.IO;

namespace CFA090307A
{
    public partial class Form3 : Form
    {
        public FileStream f_stream;
        int x, y;
        public Form3(int x,int y)
        {
            InitializeComponent();
            this.x = x;
            this.y = y;
            button_Getformula.Enabled = false;
            button_GetResult.Enabled = false;
            toolStripMenuItem_GetFormula.Enabled = false;
            toolStripMenuItem_GetResult.Enabled = false;
        }

        public bool Compute_Read()
        {
            try
            {
                f_stream = new FileStream("ComputeLog.log", FileMode.Open);
            }
            catch (System.Exception)
            {
                MessageBox.Show("打开日志文件时出错");
                return false;
            }   
            BinaryReader pw;
            try
            {
                pw = new BinaryReader(f_stream);
            }
            catch (System.Exception)
            {
                return false;
            }
            listView1.Items.Clear();
            string[] str = new string[3];
            while (true)
            {
                try
                {
                    str[0] = pw.ReadString();
                    str[1] = pw.ReadString();
                    str[2] = pw.ReadString();
                }
                catch (System.Exception)
                {
                    break;
                }
                listView1.Items.Add(new ListViewItem(str));
            }
            f_stream.Close();
            return true;
        }

        public void ComputeLog_Add(string[] log)
        {
            listView1.Items.Add(new ListViewItem(log));
        }

        private void listView1_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            if (listView1.SelectedItems.Count == 0)
            {
                button_Getformula.Enabled = false;
                button_GetResult.Enabled = false;
                toolStripMenuItem_GetFormula.Enabled = false;
                toolStripMenuItem_GetResult.Enabled = false;
            }
            else
            {
                button_Getformula.Enabled = true;
                button_GetResult.Enabled = true;
                toolStripMenuItem_GetFormula.Enabled = true;
                toolStripMenuItem_GetResult.Enabled = true;
            }
        }

        private void Form3_Load(object sender, System.EventArgs e)
        {
            Top = y;
            Left = x;
            Compute_Read();
        }

        private void button_DeleteAll_Click(object sender, System.EventArgs e)
        {
            if (MessageBox.Show("清空后将无法恢复，真的要清空吗？", "清空历史记录", MessageBoxButtons.YesNo) == DialogResult.No)
                return;
            try
            {
                f_stream = new FileStream("ComputeLog.log", FileMode.Create);
            }
            catch (System.Exception)
            {
                MessageBox.Show("打开日志文件时出错");
                return;
            }
            listView1.Items.Clear();
            f_stream.Close();
            MessageBox.Show("日志文件已清空");
        }

        private void button_Close_Click(object sender, EventArgs e)
        {
            Close();
        }

    }
}
