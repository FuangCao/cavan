using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace CFA090307A
{
    public partial class Form8 : Form
    {
        private Point[] snack;
        private Point diraction;
        private int top;
        private int H, L;
        private MyLabel[,] la;
        private int df;
        private int step;
        private int amount;
        private int x, y;

        public Form8(int x, int y)
        {
            InitializeComponent();
            H = L = 20;
            la = new MyLabel[H, L];
            snack = new Point[100];
            diraction = new Point(0, 1);
            step = 1;
            this.x = x;
            this.y = y;
        }

        private void Form8_Load(object sender, EventArgs e)
        {
            CreateLabel();
            label2.Text = "0";
            InitializeSnack();
            timer1.Interval = 300;
            timer1.Start();
            Top = y;
            Left = x;
            
        }

        private void CreateLabel()
        {
            MyLabel lb;
            for (int i = 0; i < H; i++)
            {
                for (int j = 0; j < L; j++)
                {
                    lb = new MyLabel();
                    lb.x = i;
                    lb.y = j;
                    lb.Width = panel1.Width / L;
                    lb.Height = panel1.Height / H;
                    lb.BorderStyle = BorderStyle.FixedSingle;
                    panel1.Controls.Add(lb);
                    lb.Top = panel1.Height / H * i;
                    lb.Left = panel1.Width / L * j;
                    la[i, j] = lb;
                }
            }

        }

        void Form8_GotFocus(object sender, EventArgs e)
        {
            timer1.Start();
        }

        void Form8_LostFocus(object sender, EventArgs e)
        {
            timer1.Stop();
        }

        private void CreatPoint()
        {
            Random r = new Random();
            while (true)
            {
                int x = r.Next();
                int y = r.Next();
                if (x < 0)
                    x *= -1;
                if (y < 0)
                    y *= -1;
                x %= H;
                y %= L;
                if (la[x, y].BackColor == Color.Blue)
                {
                    la[x, y].BackColor = Color.Red;
                    break;
                }
            }
        }

        void Form8_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Up)
            {
                if (diraction.X == 1 && diraction.Y == 0)
                    return;
                diraction.X = -1;
                diraction.Y = 0;
            }
            else if (e.KeyCode == Keys.Down)
            {
                if (diraction.X == -1 && diraction.Y == 0)
                    return;
                diraction.X = 1;
                diraction.Y = 0;
            }
            else if (e.KeyCode == Keys.Left)
            {
                if (diraction.X == 0 && diraction.Y == 1)
                    return;
                diraction.X = 0;
                diraction.Y = -1;
            }
            else if (e.KeyCode == Keys.Right)
            {
                if (diraction.X == 0 && diraction.Y == -1)
                    return;
                diraction.X = 0;
                diraction.Y = 1;
            }
        }

        private bool IsBody(Point p)
        {
            for (int i = 0; i <=top; i++)
            {
                if (snack[i] == p)
                    return true;
            }
            return false;
        }

        private bool SnackGo()
        {
            Point p = new Point(snack[top].X+diraction.X,snack[top].Y+diraction.Y);
            if (p.X < 0 || p.X >= H || p.Y < 0 || p.Y >= L)
                return false;
            la[snack[top].X,snack[top].Y].BackColor=Color.Red;
            if (la[p.X, p.Y].BackColor == Color.Red)
            {
                if (IsBody(p))
                    return false;
                top++;
                la[p.X, p.Y].BackColor = Color.Yellow;
                snack[top] = p;
                df+=step*10;
                amount++;
                label2.Text = df.ToString();
                if (amount==50)
                {
                    timer1.Stop();
                    if (timer1.Interval <= 50)
                    {
                        MessageBox.Show("你已玩通关了");
                        Close();  
                    }
                    else
                        timer1.Interval -= 50;
                    MessageBox.Show("第" + step.ToString() + "局结束\n" + "下面进入第" + (step + 1).ToString() + "局");
                    step++;
                    InitializeSnack();
                }
                else
                    CreatPoint();
                return true;
            }
            la[p.X, p.Y].BackColor = Color.Yellow;
            la[snack[0].X, snack[0].Y].BackColor = Color.Blue;
            for (int i = 0; i < top; i++)
            {
                snack[i] = snack[i + 1];
            }
            snack[top] = p;
            return true;
        }

        private void InitializeSnack()
        {
            for (int i = 0; i < H; i++)
            {
                for (int j = 0; j < L; j++)
                {
                    la[i, j].BackColor = Color.Blue;
                }
            }
            top = 0;
            snack[0] = new Point(0, 0);
            diraction.X = 0;
            diraction.Y = 1;
            amount = 0;
            la[0, 0].BackColor = Color.Yellow;
            CreatPoint();
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (!SnackGo())
            {
                timer1.Stop();
                MessageBox.Show("游戏结束");
                InitializeSnack();
                df = 0;
                label2.Text = "0";
                step = 1;
                timer1.Interval = 300;
            }
        }
    }

    public class MyLabel : Label
    {
        public int x, y;
    }
}
