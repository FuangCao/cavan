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
    public partial class Form4 : Form
    {
        MyDataTable T;
        String Fm;
        int x, y;

        public Form4(String Fm,int x,int y)
        {
            InitializeComponent();
            this.x = x;
            this.y = y;
            Initialize(Fm);
        }

        private void ConstructDataTable()
        {
            for (int i = 0; i < Fm.Length; i++)
            {
                if (Formula.IsLetter(Fm[i]))
                    T.Insert(Fm[i]);
            }
        }

        public void Initialize(String s)
        {
            Fm = s;
            T = new MyDataTable();
            ConstructDataTable();
            dataGridView1.Rows.Clear();
            for (int i = 0; i < T.count; i++)
            {
                dataGridView1.Rows.Add(new DataGridViewRow());
                dataGridView1.Rows[i].Cells[0].Value = i;
                dataGridView1.Rows[i].Cells[1].Value = T.Buf[i].letter + " = null";
            }
            dataGridView1.Rows[dataGridView1.NewRowIndex].Cells[0].Value = "计算结果";
            dataGridView1.Rows[dataGridView1.NewRowIndex].Cells[1].Value = Fm;
            dataGridView1.Rows[dataGridView1.NewRowIndex].ReadOnly = true;
        }

        private void dataGridView1_CellValidating(object sender, DataGridViewCellValidatingEventArgs e)
        {

            if (e.ColumnIndex == 2)
            {
                if (e.RowIndex >= T.count)
                    return;
                int n = e.RowIndex;
                String s = e.FormattedValue.ToString();
                if (String.IsNullOrEmpty(s) || s == "null")
                {
                    MessageBox.Show("数值不能为空");
                    dataGridView1.Rows[n].ErrorText = "数值不能为空";
                    e.Cancel = true;
                    return;
                }
                else if (!T.Update(s, n))
                {
                    dataGridView1.Rows[n].ErrorText = MyDataTable.Error;
                    MessageBox.Show(MyDataTable.Error);
                    e.Cancel = true;
                    return;
                }
                for (int i = 0; i < T.count; i++)
                {
                    if (T.Buf[i].exist)
                        dataGridView1.Rows[i].Cells[1].Value = T.Buf[i].letter + " = " + T.Buf[i].value.ToString();
                    else
                        dataGridView1.Rows[i].Cells[1].Value = T.Buf[i].letter + " = null";
                }
            }
        }

        private void dataGridView1_CellValidated(object sender, DataGridViewCellEventArgs e)
        {
            dataGridView1.Rows[e.RowIndex].ErrorText = String.Empty;
            String s;
            Double d;
            if (T.CreateTargetString(Fm, out s) && (Formula.Number_Compute2(s, out d)))
                dataGridView1.Rows[dataGridView1.NewRowIndex].Cells[2].Value = Formula.Number_Adjust(d);
        }

        private void button_Compute_Click(object sender, EventArgs e)
        {
            String s;
            Double d;
            if (!T.CreateTargetString(Fm, out s))
                MessageBox.Show(MyDataTable.Error);
            else if (Formula.Number_Compute(s,0, out d))
                dataGridView1.Rows[dataGridView1.NewRowIndex].Cells[2].Value = Formula.Number_Adjust(d);
        }

        private void dataGridView1_CellClick(object sender, DataGridViewCellEventArgs e)
        {
            if (e.RowIndex != dataGridView1.NewRowIndex)
            {
                if (e.ColumnIndex == 2)
                    return;
                try
                {
                    dataGridView1.CurrentCell = dataGridView1.Rows[e.RowIndex].Cells[2];
                    dataGridView1.Rows[e.RowIndex].Cells[2].DataGridView.BeginEdit(true);
                }
                catch (Exception) { }
            }
            else
            {
                dataGridView1.CurrentCell = null;
                button_Compute_Click(0, new EventArgs());
            }
        }

        private void button_Cancel_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void Form4_Load(object sender, EventArgs e)
        {
            Top = y;
            Left = x;
        }
    }

    public class DataNode
    {
        public Char letter;
        public Double value;
        public bool exist = false;
        public String Fm = "";

        public DataNode(Char a)
        {
            letter = a;
        }
    }

    public class MyDataTable
    {
        private int BM = 5;
        private int M;
        public int count = 0;
        public DataNode[] Buf;
        public int start;
        static String path;
        public static String Error;


        public MyDataTable()
        {
            M = BM;
            Buf = new DataNode[M];
        }

        private void Enlarge()  //扩大存储空间
        {
            DataNode[] old = Buf;
            Buf = new DataNode[M * 2];
            for (int i = 0; i < M; i++)
                Buf[i] = old[i];
            M *= 2;
        }

        public void Insert(Char a)
        {
            if (count == 0 || Buf[count - 1].letter < a)
            {
                if (count >= M)
                    Enlarge();
                Buf[count++] = new DataNode(a);
                return;
            }
            int i = 0;
            for (; i < count; i++)
                if (Buf[i].letter >= a)
                    break;
            if (i < count && Buf[i].letter == a)
                return;
            if (count >= M)
                Enlarge();
            for (int j = count; j > i; j--)
                Buf[j] = Buf[j - 1];
            Buf[i] = new DataNode(a);
            count++;
        }

        public int IndextOf(Char a)
        {
            for (int i = 0; i < count; i++)
                if (Buf[i].letter == a)
                    return i;
            return -1;
        }

        public bool Update(String si, int index)
        {
            Buf[index].Fm = si;
            start = index;
            path = "";
            Buf[index].exist = true;
            if (!Update(index))
            {
                Buf[index].exist = false;
                return false;
            }
            return true;
        }

        public bool Update(int index)
        {
            String s;
            if (!CreateTargetString(Buf[index].Fm, out s))
                return false;
            if (!Formula.Number_Compute2(s, out Buf[index].value))
            {
                Error = Formula.ErrorText;
                return false;
            }
            for (int i = 0; i < count; i++)
            {
                if (Buf[i].exist)
                {
                    s = Buf[i].Fm;
                    if (s.IndexOf(Buf[index].letter) != -1)
                    {
                        if (i == start)
                        {
                            Error = "存在循环引用:\n\n" + Buf[start].letter + path + " 引用 " + Buf[i].letter;
                            return false;
                        }
                        String p = path; ;
                        path = " 引用 " + Buf[i].letter +", "+ Buf[i].letter + path;
                        if (!Update(i))
                            return false;
                        path = p;
                    }
                }
            }
            return true;
        }

        public bool CreateTargetString(String si, out String out1)
        {
            out1 = "";
            for (int i = 0; i < si.Length; i++)
            {
                if (Formula.IsLetter(si[i]))
                {
                    int j = IndextOf(si[i]);
                    if (j == -1)
                    {
                        Error = "找不到符号：\"" + si[i] + "\"";
                        return false;
                    }
                    else if (Buf[j].exist)
                    {
                        if (i > 0 && (Formula.IsLetter(si[i - 1]) || Formula.IsNumber(si[i - 1])))
                            out1 += '*';
                        if (Buf[j].value < 0)
                            out1 += '(' + Buf[j].value.ToString() + ')';
                        else
                            out1 += Buf[j].value.ToString();
                    }
                    else
                    {
                        Error = "尚未给：\"" + si[i] + "\"赋值";
                        return false;
                    }
                }
                else
                    out1 += si[i];
            }
            return true;
        }
    }
}
