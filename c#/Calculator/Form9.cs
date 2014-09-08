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
    public partial class Form9 : Form
    {
        private MyLabel[,] la;
        private MyForm fa;
        private int H, L;
        private Int64 df = 0;
        private int speed = 400;
        private int rank = 1;
        private int step = 0;
        private bool pause = false;
        private int x, y;

        public Form9(int x, int y)
        {
            InitializeComponent();
            this.x = x;
            this.y = y;
        }

        private void Form9_Load(object sender, EventArgs e)
        {
            H = 20;
            L = 18;
            la = new MyLabel[H, L];
            CreateLable();
            timer1.Interval = speed;
            timer1.Start();
            fa = new MyForm(la, H, L);
            label2.Text = df.ToString();
            Top = y;
            Left = x;
        }

        void Form9_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyData == Keys.Down)
                timer1.Interval = speed;
        }

        void Form9_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyData == Keys.Space)
            {
                pause = !pause;
                if (pause)
                    timer1.Stop();
                else
                    timer1.Start();
                return;
            }
            if (pause)
                return;
            if (e.KeyData == Keys.Left)
            {
                fa.GoLeft();
            }
            else if (e.KeyData == Keys.Right)
            {
                fa.GoRight();
            }
            else if (e.KeyData == Keys.Up)
            {
                fa.GoTurn();
            }
            else if (e.KeyData == Keys.Down)
            {
                timer1.Interval = 1;
            }
            Update();
        }

        private void DeleteLine(int x)
        {
            for (int i = x; i > 0; i--)
            {
                for (int j = 0; j < L; j++)
                {
                    la[i, j].BackColor = la[i - 1, j].BackColor;
                }
            }
        }

        private void CreateLable()
        {
            MyLabel l1;
            for (int i = 0; i < H; i++)
            {
                for (int j = 0; j < L; j++)
                {
                    l1 = new MyLabel();
                    l1.x = i;
                    l1.y = j;
                    l1.BackColor = Color.Blue;
                    l1.BorderStyle = BorderStyle.FixedSingle;
                    l1.Width = panel1.Width / L;
                    l1.Height = panel1.Height / H;
                    panel1.Controls.Add(l1);
                    l1.Top = panel1.Height / H * i;
                    l1.Left = panel1.Width / L * j;
                    la[i, j] = l1;
                }
            }
        }

        private void ClearGame()
        {
            for (int i = 0; i < H; i++)
            {
                for (int j = 0; j < L; j++)
                {
                    la[i, j].BackColor = Color.Blue;
                }
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            int n = fa.GoDown();
            if (n <= 0)
            {
                if (n < 0)
                {
                    timer1.Stop();
                    MessageBox.Show("游戏结束");
                    this.Close();
                }
                bool b;
                for (int i = 0; i < H; i++)
                {
                    b = true;
                    for (int j = 0; j < L; j++)
                    {
                        if (la[i, j].BackColor == Color.Blue)
                            b = false;
                    }
                    if (b)
                    {
                        df += rank * 500;
                        step++;
                        label2.Text = df.ToString();
                        if (step % 20 == 0)
                        {
                            timer1.Stop();
                            if (speed <= 50)
                            {
                                MessageBox.Show("您已玩通关");
                                Close();
                            }
                            if (MessageBox.Show("第" + rank.ToString() + "局完成\n" + "是否进入第" + (rank + 1).ToString() + "局", "游戏升级", MessageBoxButtons.YesNo) == DialogResult.Yes)
                            {
                                rank++;
                                speed -= 50;
                                timer1.Interval = speed;
                            }
                            ClearGame();
                            timer1.Start();
                        }
                        DeleteLine(i);
                    }

                }
                fa.GetForm();
            }
        }

        private void Form9_Activated(object sender, EventArgs e)
        {
            if (pause)
                return;
            timer1.Start();
        }

        private void Form9_Deactivate(object sender, EventArgs e)
        {
            if (pause)
                return;
            timer1.Stop();
        }

    }

    public class MyForm
    {
        private int ny, nx;
        private int H, L;
        private int fh, fl;
        private int[,] ff = new int[4, 4];
        private MyLabel[,] la;

        public MyForm(MyLabel[,] lb, int H, int L)
        {
            la = lb;
            this.H = H;
            this.L = L;
            GetForm();
        }

        public void GetForm()
        {
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    ff[i, j] = 0;
                }
            }
            ny = -1;
            nx = L / 2;
            Random r = new Random();
            int n = r.Next();
            if (n < 0)
                n *= -1;
            n %= 7;
            switch (n)
            {
                case 0:
                    {
                        fh = 4;
                        fl = 1;
                        for (int i = 0; i < fh; i++)
                        {
                            for (int j = 0; j < fl; j++)
                            {
                                ff[i, j] = 0;
                            }
                        }
                        ff[0, 0] = ff[1, 0] = ff[2, 0] = ff[3, 0] = 1;
                        break;
                    }
                case 1:
                    {
                        fh = 2;
                        fl = 2;
                        for (int i = 0; i < fh; i++)
                        {
                            for (int j = 0; j < fl; j++)
                            {
                                ff[i, j] = 0;
                            }
                        }
                        ff[0, 0] = ff[0, 1] = ff[1, 0] = ff[1, 1] = 1;
                        break;
                    }
                case 2:
                    {
                        fh = 3;
                        fl = 2;
                        for (int i = 0; i < fh; i++)
                        {
                            for (int j = 0; j < fl; j++)
                            {
                                ff[i, j] = 0;
                            }
                        }
                        ff[0, 0] = ff[1, 0] = ff[2, 0] = ff[2, 1] = 1;
                        break;
                    }
                case 3:
                    {
                        fh = 3;
                        fl = 2;
                        for (int i = 0; i < fh; i++)
                        {
                            for (int j = 0; j < fl; j++)
                            {
                                ff[i, j] = 0;
                            }
                        }
                        ff[0, 1] = ff[1, 1] = ff[2, 0] = ff[2, 1] = 1;
                        break;
                    }
                case 4:
                    {
                        fh = 3;
                        fl = 2;
                        for (int i = 0; i < fh; i++)
                        {
                            for (int j = 0; j < fl; j++)
                            {
                                ff[i, j] = 0;
                            }
                        }
                        ff[0, 1] = ff[1, 0] = ff[1, 1] = ff[2, 0] = 1;
                        break;
                    }
                case 5:
                    {
                        fh = 3;
                        fl = 2;
                        for (int i = 0; i < fh; i++)
                        {
                            for (int j = 0; j < fl; j++)
                            {
                                ff[i, j] = 0;
                            }
                        }
                        ff[0, 0] = ff[1, 0] = ff[1, 1] = ff[2, 1] = 1;
                        break;
                    }
                case 6:
                    {
                        fh = 2;
                        fl = 3;
                        for (int i = 0; i < fh; i++)
                        {
                            for (int j = 0; j < fl; j++)
                            {
                                ff[i, j] = 0;
                            }
                        }
                        ff[0, 1] = ff[1, 0] = ff[1, 1] = ff[1, 2] = 1;
                        break;
                    }
            }

        }

        private bool CanDown()
        {
            if (ny == H - 1)
                return false;
            for (int j = fl - 1; j >= 0; j--)
            {
                for (int i = fh - 1; i >= 0; i--)
                {
                    if (ff[i, j] == 1)
                    {
                        if (ny - fh + i + 2 < 0)
                            break;
                        if (la[ny - fh + i + 2, nx + j].BackColor == Color.Red)
                            return false;
                        break;
                    }
                }
            }
            return true;
        }

        private bool CanLeft()
        {
            if (nx == 0)
                return false;
            for (int i = 0; i < fh; i++)
            {
                for (int j = 0; j < fl; j++)
                {
                    if (ff[i, j] == 1)
                    {
                        if (ny - fh + i + 1 < 0)
                            break;
                        if (la[ny - fh + i + 1, nx + j - 1].BackColor == Color.Red)
                            return false;
                        break;
                    }


                }
            }
            return true;
        }

        private bool CanRight()
        {
            if (nx + fl == L)
                return false;
            for (int i = 0; i < fh; i++)
            {
                for (int j = fl - 1; j >= 0; j--)
                {
                    if (ff[i, j] == 1)
                    {
                        if (ny - fh + i + 1 < 0)
                            break;
                        if (la[ny - fh + i + 1, nx + j + 1].BackColor == Color.Red)
                            return false;
                        break;
                    }
                }
            }
            return true;
        }

        public int GoDown()
        {
            if (!CanDown())
            {
                if (ny == -1)
                    return -1;
                return 0;
            }
            int cx, cy;
            for (int i = fh - 1; i >= 0; i--)
            {
                for (int j = 0; j < fl; j++)
                {
                    if (ff[i, j] == 1)
                    {
                        cx = ny - fh + i + 1;
                        cy = nx + j;
                        if (cx >= 0)
                            la[cx, cy].BackColor = Color.Blue;
                        if (cx + 1 >= 0)
                            la[cx + 1, cy].BackColor = Color.Red;
                        else
                            break;
                    }
                }
            }
            ny++;
            return 1;
        }

        public void GoLeft()
        {
            if (!CanLeft())
                return;
            int cx, cy;
            for (int i = 0; i < fh; i++)
            {
                for (int j = 0; j < fl; j++)
                {
                    if (ff[i, j] == 1)
                    {
                        cx = ny - fh + i + 1;
                        cy = nx + j;
                        if (cx < 0)
                            break;
                        la[cx, cy].BackColor = Color.Blue;
                        la[cx, cy - 1].BackColor = Color.Red;
                    }
                }
            }
            nx--;

        }

        public void GoRight()
        {
            if (!CanRight())
                return;
            int cx, cy;
            for (int i = 0; i < fh; i++)
            {
                for (int j = fl - 1; j >= 0; j--)
                {
                    if (ff[i, j] == 1)
                    {
                        cx = ny - fh + i + 1;
                        cy = nx + j;
                        if (cx < 0)
                            break;
                        la[cx, cy].BackColor = Color.Blue;
                        la[cx, cy + 1].BackColor = Color.Red;
                    }
                }
            }
            nx++;
        }

        public void GoTurn()
        {
            if (fh == fl)
                return;
            if (!CanDown())
                return;
            int cx;
            cx = fh;
            fh = fl;
            fl = cx;
            int[,] of = ff;
            ff = new int[4, 4];
            for (int i = 0; i < fl; i++)
            {
                for (int j = 0; j < fh; j++)
                {
                    if (of[i, j] == 1)
                    {
                        cx = ny - fl + i + 1;
                        if (cx >= 0)
                            la[cx, nx + j].BackColor = Color.Blue;
                    }
                    ff[j, i] = of[i, fh - 1 - j];
                }
            }
            if (fh < fl)
                nx--;
            else if (fl < fh)
                nx++;
            if (nx < 0)
                nx = 0;
            if (nx + fl >= L)
                nx = L - fl;
            if (ny >= -1)
                ny--;
            GoDown();

        }

    }
}
