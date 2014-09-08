using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Data.SqlClient;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace CFA090307A
{
    public partial class Form7 : Form
    {
        private int x, y;
        public int H, L;
        public Mybutton[,] b1 = new Mybutton[100, 100];
        public Form7(int x, int y)
        {
            InitializeComponent();
            for (int i = 3; i < 21; i++)
            {
                comboBox1.Items.Add(i);
                comboBox2.Items.Add(i);
            }
            button1.Enabled = false;
            button3.Enabled = false;
            comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 0;
            this.x = x;
            this.y = y;
        }

        private void InitializeMyButton()
        {
            for (int i = 0; i < H; i++)
            {
                for (int j = 0; j < L; j++)
                {
                    b1[i, j].BackColor = Color.Red;
                }
            }
            button3.Enabled = false;
        }

        private void RemoveMyButton()
        {
            for (int i = 0; i < H; i++)
            {
                for (int j = 0; j < L; j++)
                {
                    panel1.Controls.Remove(b1[i, j]);
                }
            }
            H = L = 0;
        }

        public void CreateMyButton(int Han, int Lie)
        {
            RemoveMyButton();
            for (int i = 0; i < Han; i++)
            {
                for (int j = 0; j < Lie; j++)
                {
                    b1[i, j] = new Mybutton();
                    b1[i, j].x = i;
                    b1[i, j].y = j;
                    panel1.Controls.Add(b1[i, j]);
                    b1[i, j].Width = panel1.Width / Lie;
                    b1[i, j].Height = panel1.Height / Han;
                    b1[i, j].BackColor = Color.Red;
                    b1[i, j].Left = panel1.Width / Lie * j;
                    b1[i, j].Top = panel1.Height / Han * i;
                    b1[i, j].Click += new EventHandler(MyButton_Click);
                }
            }
            button3.Enabled = false;
            H = Han;
            L = Lie;
        }

        private void MyButton_Click(object sender, EventArgs e)
        {
            if (button1.Enabled == true)
                return;
            Mybutton b1 = (Mybutton)(sender);
            ChangColor(b1.x, b1.y);
            ChangColor(b1.x - 1, b1.y);
            ChangColor(b1.x + 1, b1.y);
            ChangColor(b1.x, b1.y - 1);
            ChangColor(b1.x, b1.y + 1);
            if (Successed())
            {
                MessageBox.Show("您已胜出");
                for (int i = 0; i < H; i++)
                {
                    for (int j = 0; j < H; j++)
                    {
                        this.b1[i, j].BackColor = Color.Red;
                    }
                }
                InitializeMyButton();
                button3.Enabled = false;
            }
            button3.Enabled = true;
        }

        private void ChangColor(int x, int y)
        {
            if (x >= 0 && x < H && y >= 0 && y < L)
            {
                if (b1[x, y].BackColor == Color.Red)
                    b1[x, y].BackColor = Color.Blue;
                else
                    b1[x, y].BackColor = Color.Red;
            }

        }

        private bool String_Int(string si, out int out1)
        {
            out1 = 0;
            for (int i = 0; i < si.Length; i++)
            {
                if (si[i] > '9' || si[i] < '0')
                    return false;
                out1 *= 10;
                out1 += si[i] - '0';
            }
            return true;
        }

        private bool Successed()
        {
            for (int i = 0; i < H; i++)
            {
                for (int j = 0; j < L; j++)
                {
                    if (b1[i, j].BackColor == Color.Red)
                        return false;
                }
            }
            return true;
        }

        void comboBox2_TextChanged(object sender, EventArgs e)
        {
            if (comboBox1.Text.Length > 0 && comboBox2.Text.Length > 0)
                button1.Enabled = true;
            else
                button1.Enabled = false;
        }

        void comboBox1_TextChanged(object sender, EventArgs e)
        {
            if (comboBox1.Text.Length > 0 && comboBox2.Text.Length > 0)
                button1.Enabled = true;
            else
                button1.Enabled = false;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            int H, L;
            if (!String_Int(comboBox1.Text, out H) || !String_Int(comboBox2.Text, out L))
            {
                MessageBox.Show("您的输入有误");
                return;
            }
            if (H == 0)
            {
                MessageBox.Show("行数不能等于0");
                return;
            }
            if (L == 0)
            {
                MessageBox.Show("列数不能等于0");
                return;
            }
            CreateMyButton(H, L);
            button1.Enabled = false;
        }

        private void button3_Click(object sender, EventArgs e)
        {
            InitializeMyButton();
        }

        public class Mybutton : Button
        {
            public int x;
            public int y;
        }

        private void button6_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void Form7_Load(object sender, EventArgs e)
        {
            Top = y;
            Left = x;
        }
    }
}