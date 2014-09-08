using System;
using System.Drawing;
using System.IO;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace CFA090307A
{
    public partial class Form1 : Form
    {
        [DllImport("user32.dll", EntryPoint = "SendMessage")]
        public static extern int SendMessage(IntPtr hWnd, int wMsg, int wParam, int lParam);
        private const int WaitTime = 60000;

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)     //窗口加载时执行的操作
        {
            TabPage1();
            TabPage2();
            TabPage3();
            TabPage4();
            TabPage5();
            TP1_textBox_FResult.Text = DateTime.Now.ToString("yy年MM月dd日 dddd HH:mm:ss");
            timer1.Start();
        }

        private void tabControl1_Selected(object sender, TabControlEventArgs e)     //定义回车键的功能
        {
            already = false;
            timer1.Interval = 1000;
            if (e.TabPageIndex == 0)
            {
                AcceptButton = TP1_button_Compute;
                TP1_textBox_FResult.Text = DateTime.Now.ToString("yy年MM月dd日 dddd HH:mm:ss");
                timer1.Start();
            }
            else if (e.TabPageIndex == 1)
            {
                AcceptButton = TP2_button_Compute;
                TP2_textBox_MResult.Text = DateTime.Now.ToString("gg yyyy年MM月dd日 dddd tt hh:mm:ss");
                timer1.Start();
            }
            else
                timer1.Stop();
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            timer1.Interval = 1000;
            if (tabControl1.SelectedIndex == 0)
                TP1_textBox_FResult.Text = DateTime.Now.ToString("yy年MM月dd日 dddd HH:mm:ss");
            else
                TP2_textBox_MResult.Text = DateTime.Now.ToString("gg yyyy年MM月dd日 dddd tt hh:mm:ss");
        }

        #region tabPage1的代码

        private Double[] Result;
        private bool already = false;
        private String formula;
        public FileStream f_stream;
        public Form3 f3;
        public bool f3_Locked = true;
        public Queue Q_formula;
        public int dy;
        public Form2 f2;
        private Form10 f10;
        private int precision = 15;
        private int number_manner = 0;
        private int angle_manner = 0;
        private int result_mode = 0;

        private void TabPage1()     //当窗口加载时执行的操作
        {
            try
            {
                Q_formula = new Queue(100);
                Result = new Double[6];
            }
            catch (Exception)
            {
                MessageBox.Show("内存空间不足");
                Close();
            }
            删除当前行scToolStripMenuItem.Enabled = TP3_button_Remove.Enabled = false;
            contextMenuStrip2.Enabled = false;
            AcceptButton = TP1_button_Compute;
            TP1_comboBox_precision.SelectedIndex = 9;
            TP1_textBox_Formula.MouseWheel += new MouseEventHandler(TP1_textBox_Formula_MouseWheel);
            TP1_textBox_FResult.MouseWheel += new MouseEventHandler(TP1_textBox_FResult_MouseWheel);
        }

        void TP1_textBox_FResult_MouseWheel(object sender, MouseEventArgs e)
        {
            try
            {
                if (e.Delta > 0)
                    TP1_textBox_FResult.SelectionStart -= 1;
                else
                    TP1_textBox_FResult.SelectionStart += 1;
            }
            catch (Exception) { }

        }

        void TP1_textBox_Formula_MouseWheel(object sender, MouseEventArgs e)
        {
            if (e.Delta > 0)
                SendMessage(TP1_textBox_Formula.Handle, 0XB6, 0, -1);
            else
                SendMessage(TP1_textBox_Formula.Handle, 0XB6, 0, 1);
        }

        void f3_Move(object sender, EventArgs e)                //当日志窗口移动时执行的操作
        {
            f3_Locked = false;
            if (f3.Left >= Right && f3.Left - Right < 20)
            {
                f3_Locked = true;
                f3.Left = Right;
                f3.Height = Height;
                if (Math.Abs(f3.Top - Top) < 20)
                {
                    f3.Top = Top;
                    dy = 0;
                }
                else
                    dy = f3.Top - Top;
            }
        }

        private void Form1_Move(object sender, EventArgs e)     //当主窗口移动时执行的操作
        {
            if (f3 != null)
            {
                if (f3_Locked)
                {
                    f3.Top = Top + dy;
                    f3.Left = Right;
                }
                else
                {
                    if (f3.Left >= Right && f3.Left - Right < 20)
                    {
                        f3_Locked = true;
                        f3.Left = Right;
                        f3.Height = Height;
                        if (Math.Abs(f3.Top - Top) < 20)
                        {
                            f3.Top = Top;
                            dy = 0;
                        }
                        else
                            dy = f3.Top - Top;
                    }
                }
            }
        }

        private void TP1_button_Up_Click(object sender, EventArgs e)            //单击“上翻”键时执行的操作
        {
            TP1_textBox_Formula.Focus();
            String s;
            if (TP1_textBox_Formula.TextLength == 0)
            {
                if (!Q_formula.GetSeekData(out s))
                    return;
            }
            else
            {
                if (!Q_formula.After_GetData(out s))
                    return;
            }
            TP1_textBox_Formula.Clear();
            TP1_textBox_Formula.SelectedText = s;
            TP1_textBox_Formula.Select(TP1_textBox_Formula.SelectionStart, 0);
        }

        private void TP1_button_Down_Click(object sender, EventArgs e)          //单击“下翻”键时执行的操作
        {
            TP1_textBox_Formula.Focus();
            String s;
            if (TP1_textBox_Formula.TextLength == 0)
            {
                if (!Q_formula.GetSeekData(out s))
                    return;
            }
            else
            {
                if (!Q_formula.Front_GetData(out s))
                    return;
            }
            TP1_textBox_Formula.Clear();
            TP1_textBox_Formula.SelectedText = s;
            TP1_textBox_Formula.Select(TP1_textBox_Formula.SelectionStart, 0);
        }

        private void TP1_button_Compute_Click(object sender, EventArgs e)       //单击“=”键时执行的操作
        {
            TP1_textBox_Formula.Focus();
            String f = CreateFormula(TP1_textBox_Formula.Text);
            if (formula != f)
            {
                formula = f;
                Double d;
                if (Formula.Number_Compute(formula, angle_manner, out d))
                {
                    Result[0] = d;
                    Q_formula.InQueue2(formula);
                    try
                    {
                        ComputeLog_Add(new String[] { DateTime.Now.ToString("yyyy年MM月dd日 HH:mm:ss"), formula, Result[0].ToString() });
                    }
                    catch (Exception)
                    {
                        MessageBox.Show("内存空间不足");
                        return;
                    }
                }
                else
                {
                    formula = null;
                    return;
                }
            }
            timer1.Stop();
            timer1.Interval = WaitTime;
            TP1_textBox_FResult.Text = "= " + Formula.Number_Adjust(Result[0], number_manner, precision);
            result_mode = 0;
            already = true;
            timer1.Start();
        }

        private void TP1_button_abDivc_Click(object sender, EventArgs e)
        {
            if (!already)
            {
                MessageBox.Show("请先进行计算");
                return;
            }
            String s;
            if (Math.Round(Result[0], 14) == (int)Result[0] || result_mode > 1 || Math.Abs(Result[0]) <= 1 && result_mode > 0)
            {
                s = Formula.Number_Adjust(Result[0], number_manner, precision);
                result_mode = 0;
            }
            else if (Formula.D_F(Result[0], result_mode, out s))
                result_mode++;
            else
            {
                MessageBox.Show("无法转换");
                return;
            }
            timer1.Stop();
            timer1.Interval = WaitTime;
            TP1_textBox_FResult.Text = "= " + s;
            timer1.Start();
        }

        private void TP1_button_Angle_Click(object sender, EventArgs e)         //单击三角函数键时执行的操作
        {
            TP1_textBox_Formula.Focus();
            if (already)
                TP1_textBox_Formula.Clear();
            TP1_textBox_Formula.SelectedText = ((Button)sender).Text;
            TP1_textBox_Formula.Select(TP1_textBox_Formula.SelectionStart, 0);
        }

        private void TP1_button_Number_Click(object sender, EventArgs e)        //单击输入数字的键时执行的操作
        {
            TP1_textBox_Formula.Focus();
            if (already)
                TP1_textBox_Formula.Clear();
            String s = ((Button)sender).Text;
            if (s == "π")
                TP1_textBox_Formula.SelectedText = "PI";
            else if (s == "·")
            {
                if (TP1_textBox_Formula.SelectionStart == 0 || !Formula.IsNumber(TP1_textBox_Formula.Text[(TP1_textBox_Formula.SelectionStart - 1)]))
                    TP1_textBox_Formula.SelectedText = "0.";
                else
                    TP1_textBox_Formula.SelectedText = ".";
            }
            else if ((s == "(" || s == ")") && TP1_textBox_Formula.SelectionLength != 0)
            {
                int a = TP1_textBox_Formula.SelectionStart + TP1_textBox_Formula.SelectionLength + 1;
                TP1_textBox_Formula.Select(TP1_textBox_Formula.SelectionStart, 0);
                TP1_textBox_Formula.SelectedText = "(";
                TP1_textBox_Formula.Select(a, 0);
                TP1_textBox_Formula.SelectedText = ")";
            }
            else
                TP1_textBox_Formula.SelectedText = s;
            TP1_textBox_Formula.Select(TP1_textBox_Formula.SelectionStart, 0);
        }

        private void TP1_button_Operator1_Click(object sender, EventArgs e)     //单击输入运算符的键时执行的操作
        {
            TP1_textBox_Formula.Focus();
            if (already)
            {
                TP1_textBox_Formula.Clear();
                TP1_textBox_Formula.SelectedText = "ANS";
                TP1_textBox_Formula.Select(TP1_textBox_Formula.SelectionStart, 0);
            }
            String s = ((Button)sender).Text;
            if (s == "Mod")
                TP1_textBox_Formula.SelectedText = "%";
            else if (s == "nCr")
                TP1_textBox_Formula.SelectedText = "C";
            else if (s == "nAr")
                TP1_textBox_Formula.SelectedText = "A";
            else if (s == "X²")
                TP1_textBox_Formula.SelectedText = "²";
            else if (s == "X³")
                TP1_textBox_Formula.SelectedText = "³";
            else if (s == "Y^X")
                TP1_textBox_Formula.SelectedText = "^";
            else if (s == "n!")
                TP1_textBox_Formula.SelectedText = "!";
            else if (s == "+" && TP1_textBox_Formula.SelectionStart == 0)
                TP1_textBox_Formula.SelectedText = "0" + s;
            else
                TP1_textBox_Formula.SelectedText = s;
            TP1_textBox_Formula.Select(TP1_textBox_Formula.SelectionStart, 0);
        }

        private void TP1_button_Clear_Click(object sender, EventArgs e)         //单击“清除”键时执行的操作
        {
            TP1_textBox_Formula.Focus();
            TP1_textBox_Formula.Clear();
            TP1_textBox_FResult.Clear();
            TP1_textBox_FResult.Text = DateTime.Now.ToString("yy年MM月dd日 dddd HH:mm:ss");
            timer1.Interval = 1000;
        }

        private void TP1_button_Result_Click(object sender, EventArgs e)        //单击“结果”键时执行的操作
        {
            if (already)
                TP1_textBox_Formula.Clear();
            TP1_textBox_Formula.SelectedText = "ANS";
            TP1_textBox_Formula.Select(TP1_textBox_Formula.SelectionStart, 0);
            timer1.Stop();
            timer1.Interval = WaitTime;
            TP1_textBox_FResult.Text = "ANS= " + Formula.Number_Adjust(Result[0], number_manner, precision);
            TP1_textBox_Formula.Focus();
            timer1.Start();
        }

        private void TP1_button_Back_Click(object sender, EventArgs e)          //单击“Back”键时执行的操作
        {
            TP1_textBox_Formula.Focus();
            if (TP1_textBox_Formula.SelectionLength == 0)
            {
                if (TP1_textBox_Formula.SelectionStart == 0)
                    return;
                TP1_textBox_Formula.Select(TP1_textBox_Formula.SelectionStart - 1, 1);
            }
            TP1_textBox_Formula.SelectedText = "";
        }

        private void TP1_button_Operator2_Click(object sender, EventArgs e)     //单击输入运算符的键时执行的操作
        {
            TP1_textBox_Formula.Focus();
            if (already)
                TP1_textBox_Formula.Clear();
            String s = ((Button)sender).Text;
            if (s == "Exp")
                TP1_textBox_Formula.SelectedText = "e^";
            else if (s == "Log")
            {
                TP1_textBox_Formula.SelectedText = "Log(,";
                TP1_textBox_Formula.SelectionStart = TP1_textBox_Formula.SelectionStart - 1;
            }
            else if (s == "Ln")
                TP1_textBox_Formula.SelectedText = "Ln";
            else
                TP1_textBox_Formula.SelectedText = s;
            TP1_textBox_Formula.Select(TP1_textBox_Formula.SelectionStart, 0);
        }

        private void TP1_button_Close_Click(object sender, EventArgs e)         //单击“关闭”键时执行的操作
        {
            Close();
        }

        private void TP1_button_memorizer_Click(object sender, EventArgs e)     //单击存储器键时执行的操作
        {
            String s = ((Button)sender).Text;
            s = "M" + s[s.Length - 1];
            int i = s[1] - 'A' + 1;
            timer1.Stop();
            if (radioButton_TP1_write.Checked)
            {
                Result[i] = Result[0];
                timer1.Interval = WaitTime;
                TP1_textBox_FResult.Text = "ANS ☞ " + s + "= " + Formula.Number_Adjust(Result[0], number_manner, precision);
            }
            else
            {
                if (already)
                    TP1_textBox_Formula.Clear();
                TP1_textBox_Formula.SelectedText = s;
                TP1_textBox_Formula.Select(TP1_textBox_Formula.SelectionStart, 0);
                timer1.Interval = WaitTime;
                TP1_textBox_FResult.Text = s + "= " + Formula.Number_Adjust(Result[i], number_manner, precision);
                TP1_textBox_Formula.Focus();
            }
            TP1_textBox_Formula.Focus();
            timer1.Start();
        }

        private void TP1_button_switch_Click(object sender, EventArgs e)        //单击“进制转换”键时执行的操作
        {
            TP1_textBox_Formula.Focus();
            if (f2 == null || f2.IsDisposed)
            {
                try
                {
                    f2 = new Form2(Right, Top);
                }
                catch (Exception)
                {
                    MessageBox.Show("内存空间不足");
                    return;
                }
                f2.Show();
            }
            f2.textBox_decimal.Focus();
            f2.textBox_decimal.Text = Formula.Number_Adjust(Result[0]);
            f2.textBox_decimal.SelectionStart = f2.textBox_decimal.TextLength;
        }

        private void TP1_button_ComputerLog_Click(object sender, EventArgs e)   //单击“日志”键时执行的操作
        {
            TP1_textBox_Formula.Focus();
            if (f3 == null || f3.IsDisposed)
            {
                try
                {
                    f3 = new Form3(Right, Top);
                    f3.Move += new EventHandler(f3_Move);
                    f3.listView1.DoubleClick += new EventHandler(TP1_listView1_DoubleClick);
                    f3.button_Getformula.Click += new EventHandler(TP1_button_Getformula_Click);
                    f3.button_GetResult.Click += new EventHandler(TP1_button_GetResult_Click);
                    f3.button_Hide.Click += new EventHandler(TP1_button_ComputerLog_Click);
                    f3.toolStripMenuItem_GetFormula.Click += new EventHandler(TP1_button_Getformula_Click);
                    f3.toolStripMenuItem_GetResult.Click += new EventHandler(TP1_button_GetResult_Click);
                    f3.ToolStripMenuItem_Hide.Click += new EventHandler(TP1_button_ComputerLog_Click);
                }
                catch (Exception)
                {
                    MessageBox.Show("内存空间不足");
                    return;
                }
                f3.Height = Height;
                f3.Show();
            }
            else
            {
                if (f3.Visible == false)
                {
                    f3.Top = Top;
                    f3.Left = Right;
                    f3.Visible = true;
                }
                else
                {
                    if (f3_Locked && f3.Top == Top)
                    {
                        for (int i = f3.Left; i > Left; i -= 80)
                        {
                            System.Threading.Thread.Sleep(50);
                            f3.Left = i;
                        }
                    }
                    f3.Visible = false;
                }
            }
        }

        private void TP1_button_GetResult_Click(object sender, EventArgs e)     //单击f3下的“获取结果”键时执行的操作
        {
            TP1_textBox_Formula.Focus();
            if (already)
                TP1_textBox_Formula.Clear();
            ListView l1 = f3.listView1;
            TP1_textBox_Formula.SelectedText = l1.SelectedItems[0].SubItems[2].Text;
            TP1_textBox_Formula.Select(TP1_textBox_Formula.SelectionStart, 0);
        }

        private void TP1_button_Getformula_Click(object sender, EventArgs e)    //单击f3下的“获取算式”键时执行的操作
        {
            TP1_textBox_Formula.Focus();
            try
            {
                TP1_listView1_DoubleClick(0, new EventArgs());
            }
            catch (Exception)
            {
                MessageBox.Show("内存空间不足");
                return;
            }
        }

        private void TP1_textBox_Formula_MouseDown(object sender, MouseEventArgs e)
        {
            try
            {
                TP1_textBox_Formula_TextChanged(0, new EventArgs());
            }
            catch (Exception)
            {
                MessageBox.Show("内存空间不足");
                return;
            }
        }

        private void TP1_textBox_Formula_TextChanged(object sender, EventArgs e)        //当算式被更改时执行的操作
        {
            if (already)
            {
                already = false;
                TP1_textBox_FResult.Clear();
                timer1.Interval = 1000;
            }
        }

        private void TP1_radioButton_angle_CheckedChanged(object sender, EventArgs e)   //单击“角度”时执行的操作
        {
            TP1_textBox_Formula.Focus();
            if (TP1_textBox_Formula.TextLength == 0)
                return;
            timer1.Stop();
            if (TP1_radioButton_angle.Checked)
            {
                if (!already)
                {
                    if (!Formula.Number_Compute(CreateFormula(TP1_textBox_Formula.Text), angle_manner, out Result[0]))
                    {
                        angle_manner = 0;
                        return;
                    }
                }
                else
                    TP1_textBox_Formula.Text = Formula.Number_Adjust(Result[0]);
                already = true;
                formula = null;
                Result[0] = Formula.Radian_Angle(Result[0]);
                timer1.Interval = WaitTime;
                TP1_textBox_FResult.Text = "= " + Formula.Number_Adjust(Result[0], number_manner, precision) + "(度)";
                angle_manner = 0;
            }
            else
            {
                if (!already)
                {
                    if (!Formula.Number_Compute(CreateFormula(TP1_textBox_Formula.Text), angle_manner, out Result[0]))
                    {
                        angle_manner = 1;
                        return;
                    }
                }
                else
                    TP1_textBox_Formula.Text = Formula.Number_Adjust(Result[0]);
                already = true;
                formula = null;
                Result[0] = Formula.Angle_Radian(Result[0]);
                timer1.Interval = WaitTime;
                TP1_textBox_FResult.Text = "= " + Formula.Number_Adjust(Result[0], number_manner, precision) + "(弧度)";
                angle_manner = 1;
            }
            TP1_textBox_Formula.Select(TP1_textBox_Formula.TextLength, 0);
            timer1.Start();

        }

        private void TP1_radioButton_common_CheckedChanged(object sender, EventArgs e)  //单击“普通”时执行的操作
        {
            if (TP1_radioButton_common.Checked)
                number_manner = 0;
            else
                number_manner = 1;
            if (already)
            {
                timer1.Stop();
                timer1.Interval = WaitTime;
                TP1_textBox_FResult.Text = "= " + Formula.Number_Adjust(Result[0], number_manner, precision);
                timer1.Start();
            }
            TP1_textBox_Formula.Focus();
        }

        private void TP1_comboBox_precision_TextChanged(object sender, EventArgs e)     //当精度时更改时执行的操作
        {
            if (TP1_comboBox_precision.Text.Length == 0)
            {
                MessageBox.Show("请输入精度");
                return;
            }
            int a;
            if (String_Int(TP1_comboBox_precision.Text, out a))
            {
                if (a < 0)
                {
                    MessageBox.Show("小数位数应≥0");
                    TP1_comboBox_precision.Text = precision.ToString();
                }
                else if (a > 15)
                {
                    MessageBox.Show("\"Double\"型数的精度不能大于15");
                    TP1_comboBox_precision.Text = precision.ToString();
                }
                else
                {
                    precision = a;
                    if (already)
                    {
                        timer1.Stop();
                        timer1.Interval = WaitTime;
                        TP1_textBox_FResult.Text = "= " + Formula.Number_Adjust(Result[0], number_manner, precision);
                        timer1.Start();
                    }
                }
            }
            else
                TP1_comboBox_precision.Text = precision.ToString();
            TP1_comboBox_precision.Select(TP1_comboBox_precision.Text.Length, 0);
        }

        private void TP1_listView1_DoubleClick(object sender, EventArgs e)          //在f3下双击时执行的操作
        {
            TP1_textBox_Formula.Focus();
            if (already)
                TP1_textBox_Formula.Clear();
            ListView l1 = f3.listView1;
            TP1_textBox_Formula.SelectedText = l1.SelectedItems[0].SubItems[1].Text;
            TP1_textBox_Formula.Select(TP1_textBox_Formula.SelectionStart, 0);
        }

        public bool ComputeLog_Add(String[] log)        //添加一条日志的操作
        {
            if (f3 != null && !f3.IsDisposed)
                f3.ComputeLog_Add(log);
            try
            {
                f_stream = new FileStream("ComputeLog.log", FileMode.OpenOrCreate);
            }
            catch (Exception)
            {
                return false;
            }
            BinaryWriter pw;
            try
            {
                pw = new BinaryWriter(f_stream);
            }
            catch (Exception)
            {
                MessageBox.Show("内存空间不足");
                return false;
            }
            f_stream.Seek(0, SeekOrigin.End);
            for (int i = 0; i < 3; i++)
                pw.Write(log[i]);
            pw.Close();
            f_stream.Close();
            return true;
        }

        public bool String_Int(String s, out int out1)  //把一个只含有数字字符字符串转换为一个整数的操作
        {
            out1 = 0;
            for (int i = 0; i < s.Length; i++)
            {
                if (s[i] >= '0' && s[i] <= '9')
                {
                    out1 *= 10;
                    out1 += s[i] - '0';
                }
                else
                {
                    MessageBox.Show("存在非数字字符：\"" + s[i] + "\"");
                    return false;
                }
            }
            return true;
        }

        private void button_UnitSwitch_Click(object sender, EventArgs e)    //单击“单位换算”按键时执行的的操作
        {
            if (f10 == null || f10.IsDisposed)
            {
                try
                {
                    f10 = new Form10(Right, Top);
                }
                catch (Exception)
                {
                    MessageBox.Show("内存空间不足");
                    return;
                }
                f10.Show();
            }
            f10.textBox_Value1.Text = Formula.Number_Adjust(Result[0]);
        }

        private bool IsOperator(Char a)
        {
            String str = "+-*×/÷%^AC√";
            for (int i = 0; i < str.Length; i++)
                if (a == str[i])
                    return true;
            return false;
        }

        private String CreateFormula(String si)
        {
            String s = "";
            for (int i = 0; i < si.Length; i++)
            {
                if (si[i] == 'A')
                {
                    if (i + 2 < si.Length && si[i + 1] == 'N' && si[i + 2] == 'S')
                    {
                        s += "[" + Result[0] + "]";
                        i += 2;
                        continue;
                    }
                }
                else if (si[i] == 'M')
                {
                    if (i + 1 < si.Length && si[i + 1] >= 'A' && si[i + 1] <= 'Z')
                    {
                        s += "[" + Result[si[i + 1] - 'A' + 1] + "]";
                        i++;
                        continue;
                    }
                }
                s += si[i];
            }
            return s;
        }

        #endregion

        #region tabPage2的代码

        private Queue Q_Multinomial;
        private String M_Result = "";
        private Form4 f4;

        private void TabPage2() //当窗口加载时执行的操作
        {
            try
            {
                Q_Multinomial = new Queue(100);
            }
            catch (Exception)
            {
                MessageBox.Show("内存空间不足");
                Close();
            }
            TP2_textBox_MResult.MouseWheel += new MouseEventHandler(TP2_textBox_MResult_MouseWheel);
            TP2_textBox_Formula.MouseWheel += new MouseEventHandler(TP2_textBox_Formula_MouseWheel);

        }

        void TP2_textBox_Formula_MouseWheel(object sender, MouseEventArgs e)
        {
            if (e.Delta > 0)
                SendMessage(TP2_textBox_Formula.Handle, 0XB6, 0, -1);
            else
                SendMessage(TP2_textBox_Formula.Handle, 0XB6, 0, 1);
        }

        void TP2_textBox_MResult_MouseWheel(object sender, MouseEventArgs e)
        {
            if (e.Delta > 0)
                SendMessage(TP2_textBox_MResult.Handle, 0XB6, 0, -1);
            else
                SendMessage(TP2_textBox_MResult.Handle, 0XB6, 0, 1);
        }

        private void TP2_button_Compute_Click(object sender, EventArgs e)   //单击“=”键时执行的操作
        {
            if (!already)
            {
                if (MultinomialC.Multinomial_Compute(TP2_textBox_Formula.Text, out M_Result))
                {
                    String s;
                    if (!Q_Multinomial.GetHeadData(out s) || s != TP2_textBox_Formula.Text)
                        Q_Multinomial.InQueue2(TP2_textBox_Formula.Text);
                    already = true;
                }
                else
                    return;
                
            }
            timer1.Stop();
            timer1.Interval = WaitTime;
            TP2_textBox_MResult.Text = "= " + M_Result;
            TP2_textBox_Formula.Focus();
            timer1.Start();
        }

        private void TP2_button_Input_Click(object sender, EventArgs e)     //单击输入多项式的键时执行的操作
        {
            TP2_textBox_Formula.Focus();
            String s = ((Button)sender).Text;
            if (s == "·")
                TP2_textBox_Formula.SelectedText = ".";
            else if (s == "X^n")
                TP2_textBox_Formula.SelectedText = "^";
            else if (s == "X^2")
                TP2_textBox_Formula.SelectedText = "^2";
            else if (s == "X^3")
                TP2_textBox_Formula.SelectedText = "^3";
            else if (s == "Mod")
                TP2_textBox_Formula.SelectedText = "%";
            else if (s == "n!")
                TP2_textBox_Formula.SelectedText = "!";
            else
                TP2_textBox_Formula.SelectedText = s;
            TP2_textBox_Formula.Select(TP2_textBox_Formula.SelectionStart, 0);
        }

        private void TP2_button_Back_Click(object sender, EventArgs e)      //单击“Back”键时执行的操作
        {
            TP2_textBox_Formula.Focus();
            if (TP2_textBox_Formula.SelectionLength == 0)
            {
                if (TP2_textBox_Formula.SelectionStart == 0)
                    return;
                TP2_textBox_Formula.Select(TP2_textBox_Formula.SelectionStart - 1, 1);
            }
            TP2_textBox_Formula.SelectedText = "";
        }

        private void TP2_button_Clear_Click(object sender, EventArgs e)     //单击“清除”键时执行的操作
        {
            TP2_textBox_Formula.Focus();
            TP2_textBox_Formula.Clear();
            TP2_textBox_MResult.Clear();
            TP2_textBox_MResult.Text = DateTime.Now.ToString("gg yyyy年MM月dd日 dddd tt hh:mm:ss");
            timer1.Interval = 1000;
        }

        private void TP2_button_Result_Click(object sender, EventArgs e)    //单击“结果”键时执行的操作
        {
            TP2_textBox_Formula.Focus();
            if (already)
                TP2_textBox_Formula.Clear();
            TP2_textBox_Formula.SelectedText = M_Result;
            TP2_textBox_Formula.Select(TP2_textBox_Formula.SelectionStart, 0);
        }

        private void TP2_button_Evaluation_Click(object sender, EventArgs e)    //单击“求值”键时执行的操作
        {
            TP2_textBox_Formula.Focus();
            if (TP2_textBox_Formula.TextLength == 0)
            {
                MessageBox.Show("请先输入一个多项式");
                return;
            }
            if (f4 == null || f4.IsDisposed)
            {
                try
                {
                    f4 = new Form4(TP2_textBox_Formula.Text, Right, Top);
                }
                catch (Exception)
                {
                    MessageBox.Show("内存空间不足");
                    return;
                }
                f4.Show();
            }
            else
                f4.Initialize(TP2_textBox_Formula.Text);
        }

        private void TP2_button_Up_Click(object sender, EventArgs e)            //单击“上翻”键时执行的操作
        {
            TP2_textBox_Formula.Focus();
            String s;
            if (TP2_textBox_Formula.TextLength == 0)
            {
                if (!Q_Multinomial.GetSeekData(out s))
                    return;
            }
            else
            {
                if (!Q_Multinomial.After_GetData(out s))
                    return;
            }
            TP2_textBox_Formula.Clear();
            TP2_textBox_Formula.SelectedText = s;
            TP2_textBox_Formula.Select(TP2_textBox_Formula.SelectionStart, 0);
        }

        private void TP2_button_Down_Click(object sender, EventArgs e)          //单击“下翻”键时执行的操作
        {
            TP2_textBox_Formula.Focus();
            String s;
            if (TP2_textBox_Formula.TextLength == 0)
            {
                if (!Q_Multinomial.GetSeekData(out s))
                    return;
            }
            else
            {
                if (!Q_Multinomial.Front_GetData(out s))
                    return;
            }
            TP2_textBox_Formula.Clear();
            TP2_textBox_Formula.SelectedText = s;
            TP2_textBox_Formula.Select(TP2_textBox_Formula.SelectionStart, 0);
        }

        private void TP2_textBox_Formula_TextChanged(object sender, EventArgs e)    //当多项式改变时执行的操作
        {
            if (already)
            {
                already = false;
                TP2_textBox_MResult.Clear();
                timer1.Interval = 1000;
            }
        }

        private void TP2_textBox_Formula_MouseDown(object sender, MouseEventArgs e)
        {
            try
            {
                TP2_textBox_Formula_TextChanged(0, new EventArgs());
            }
            catch (Exception)
            {
                MessageBox.Show("内存空间不足");
                return;
            }
        }
        #endregion

        #region tabPage3的代码

        public MyTable table;

        private void TabPage3() //在窗口加载时执行的操作
        {
            try
            {
                table = new MyTable(10);
            }
            catch (Exception)
            {
                MessageBox.Show("内存空间不足");
                Close();
            }
        }

        private void TP3_dataGridView1_SelectionChanged(object sender, EventArgs e) //但选择改变时执行的操作
        {
            if (TP3_dataGridView1.SelectedRows.Count == 0 || TP3_dataGridView1.SelectedRows.Count == 1 && TP3_dataGridView1.SelectedRows[0].Index == TP3_dataGridView1.NewRowIndex)
                删除当前行scToolStripMenuItem.Enabled = TP3_button_Remove.Enabled = false;
            else
                删除当前行scToolStripMenuItem.Enabled = TP3_button_Remove.Enabled = true;
        }

        private void TP3_dataGridView1_DefaultValuesNeeded(object sender, DataGridViewRowEventArgs e)       //添加一行时做初始化
        {
            e.Row.Cells[0].Value = e.Row.Index;
            table.Add();
            contextMenuStrip2.Enabled = true;
        }

        private void TP3_dataGridView1_CellValidating(object sender, DataGridViewCellValidatingEventArgs e) //单元格内容检查
        {
            if (e.ColumnIndex == 0)
                return;
            String s = e.FormattedValue.ToString();
            if (e.ColumnIndex == 1)
            {
                if (s == "" || s == "null")
                {
                    TP3_dataGridView1.Rows[e.RowIndex].ErrorText = "\"数值\"不能为空";
                    MessageBox.Show("\"数值\"不能为空");
                    contextMenuStrip2.Enabled = false;
                    e.Cancel = true;
                    return;
                }
                Double d1;
                if (!Formula.Number_Compute(s, 0, out d1))
                {
                    TP3_dataGridView1.Rows[e.RowIndex].ErrorText = Formula.ErrorText;
                    contextMenuStrip2.Enabled = false;
                    e.Cancel = true;
                    return;
                }
                else
                    table.Update1(e.RowIndex, d1);
            }
            else
            {
                if (s != "" && s != "null")
                {
                    Double d1;
                    if (!Formula.Number_Compute(s, 0, out d1))
                    {
                        TP3_dataGridView1.Rows[e.RowIndex].ErrorText = Formula.ErrorText;
                        contextMenuStrip2.Enabled = false;
                        e.Cancel = true;
                        return;
                    }
                    if (d1 > 1 || d1 < 0)
                    {
                        MessageBox.Show("概率应大于等于\"0\"小于\"1\" ");
                        TP3_dataGridView1.Rows[e.RowIndex].ErrorText = "概率应大于等于\"0\"小于\"1\" ";
                        contextMenuStrip2.Enabled = false;
                        e.Cancel = true;
                        return;
                    }
                    table.Update2(e.RowIndex, d1);
                }
                else
                    table.Update2(e.RowIndex, -1);
            }
        }

        private void TP3_dataGridView1_RowValidating(object sender, DataGridViewCellCancelEventArgs e)      //行的检查
        {
            Object O = TP3_dataGridView1.Rows[e.RowIndex].Cells[1].Value;
            if (O == null || String.IsNullOrEmpty(O.ToString()))
            {
                TP3_dataGridView1.Rows[e.RowIndex].ErrorText = "\"数值\"不能为空";
                MessageBox.Show("\"数值\"不能为空");
                contextMenuStrip2.Enabled = false;
                e.Cancel = true;
                return;
            }
        }

        private void TP3_button_Remove_Click(object sender, EventArgs e)    //单击“删除”键时执行的操作
        {
            int i = TP3_dataGridView1.SelectedRows[0].Index;
            try
            {
                TP3_dataGridView1.Rows.Remove(TP3_dataGridView1.SelectedRows[0]);
            }
            catch (Exception) { return; }
            table.Remove(i);
            int length = TP3_dataGridView1.RowCount - 1;
            for (; i < length; i++)
                TP3_dataGridView1[0, i].Value = i;
        }

        private void TP3_button_Clear_Click(object sender, EventArgs e)     //单击“清除”键时执行的操作
        {
            try
            {
                TP3_dataGridView1.Rows.Clear();
                table.Clear();
                contextMenuStrip2.Enabled = false;
            }
            catch (Exception) { }
        }

        private void TP3_button_Sum_Click(object sender, EventArgs e)       //单击“求和”键时执行的操作
        {
            Double d;
            if (TP3_dataGridView1.NewRowIndex == 0)
            {
                MessageBox.Show("表中没有数据");
                return;
            }
            if (table.SumN(1, out d))
                TP3_textBoxP_Result.Text = "∑x= " + Formula.Number_Adjust(d);
            else
                MessageBox.Show(MyTable.Error);
        }

        private void TP3_button_SumXn_Click(object sender, EventArgs e)     //单击“求X^n的和”键时执行的操作
        {
            if (TP3_dataGridView1.NewRowIndex == 0)
            {
                MessageBox.Show("表中没有数据");
                return;
            }
            Double n;
            if (TP3_textBoxP_Result.TextLength == 0 || !Formula.Number_Compute2(TP3_textBoxP_Result.Text, out n))
            {
                MessageBox.Show("请在上面的编辑框中输入\"n\"的值");
                TP3_textBoxP_Result.Clear();
                TP3_textBoxP_Result.Focus();
                return;
            }
            Double d;
            if (table.SumN(n, out d))
                TP3_textBoxP_Result.Text = "∑x^" + n + "= " + Formula.Number_Adjust(d);
            else
                MessageBox.Show(MyTable.Error);
        }

        private void TP3_button_Average_Click(object sender, EventArgs e)   //单击“平均值”键时执行的操作
        {
            if (TP3_dataGridView1.NewRowIndex == 0)
            {
                MessageBox.Show("表中没有数据");
                return;
            }
            Double d;
            if (table.Average(1, out d))
                TP3_textBoxP_Result.Text = "AVG(x)= " + Formula.Number_Adjust(d);
            else
                MessageBox.Show(MyTable.Error);
        }

        private void TP3_button_Expectation_Click(object sender, EventArgs e)       //单击“期望”键时执行的操作
        {
            if (TP3_dataGridView1.NewRowIndex == 0)
            {
                MessageBox.Show("表中没有数据");
                return;
            }
            Double d;
            if (table.Expectation(out d))
                TP3_textBoxP_Result.Text = "Ex= " + Formula.Number_Adjust(d);
            else
                MessageBox.Show(MyTable.Error);
        }

        private void TP3_button_Variance_Click(object sender, EventArgs e)          //单击“方差”键时执行的操作
        {
            if (TP3_dataGridView1.NewRowIndex == 0)
            {
                MessageBox.Show("表中没有数据");
                return;
            }
            Double d;
            if (table.Variance(out d))
                TP3_textBoxP_Result.Text = "Sx= " + Formula.Number_Adjust(d);
            else
                MessageBox.Show(MyTable.Error);
        }

        private void TP3_button_StandardDeviation_Click(object sender, EventArgs e) //单击“标准差”键时执行的操作
        {
            if (TP3_dataGridView1.NewRowIndex == 0)
            {
                MessageBox.Show("表中没有数据");
                return;
            }
            Double d;
            if (table.StandardDeviation(out d))
                TP3_textBoxP_Result.Text = "δx= " + Formula.Number_Adjust(d);
            else
                MessageBox.Show(MyTable.Error);
        }

        private void TP3_dataGridView1_CellValidated(object sender, DataGridViewCellEventArgs e)    //单元格检查成功时执行的操作
        {
            TP3_textBoxP_Result.Clear();
            TP3_dataGridView1.Rows[e.RowIndex].ErrorText = String.Empty;
            contextMenuStrip2.Enabled = true;
        }

        #endregion

        #region tabPage4的代码

        private int CB_Height = 20;
        private int CB_With = 50;
        private MyColorBar[] ColorBar;
        private MyColorLoop[] Loop;
        private MyColorLoop CurrentLoop;
        private Panel TP4_panel_ColorBar;
        private Color OldColor;
        private int Ln;
        private bool IsLTR = false;
        Form5 f5;
        Form6 f6;

        private void TabPage4()         //窗口加载时执行的初始化函数
        {
            try
            {
                ColorBar = new MyColorBar[13];
                Loop = new MyColorLoop[5];
                TP4_panel_ColorBar = new Panel();
            }
            catch (Exception)
            {
                MessageBox.Show("内存空间不足");
                Close();
            }
            for (int i = 0; i < 5; i++)
            {
                try
                {
                    Loop[i] = new MyColorLoop(i);
                    Loop[i].MouseEnter += new EventHandler(Loop_MouseEnter);
                    Loop[i].BackColorChanged += new EventHandler(Loop_BackColorChanged);
                }
                catch (Exception)
                {
                    MessageBox.Show("内存空间不足");
                    Close();
                }
                Loop[i].Height = TP4_panel_Resistor.Height;
                Loop[i].Width = CB_With;
                Loop[i].Top = 0;
                Loop[i].Text = Color_String(Loop[i].BackColor);
                if (Loop[i].BackColor == Color.White || Loop[i].BackColor == Color.Yellow)
                    Loop[i].ForeColor = Color.Black;
                else
                    Loop[i].ForeColor = Color.White;
            }
            TP4_panel_Resistor.Controls.AddRange(Loop);
            for (int i = 0; i < 13; i++)
            {
                try
                {
                    ColorBar[i] = new MyColorBar(i);
                    ColorBar[i].MouseClick += new MouseEventHandler(ColorBar_MouseClick);
                    ColorBar[i].MouseEnter += new EventHandler(ColorBar_MouseEnter);
                }
                catch (Exception)
                {
                    MessageBox.Show("内存空间不足");
                    Close();
                }
                ColorBar[i].Width = CB_With;
                ColorBar[i].Height = CB_Height;
                ColorBar[i].Left = 0;
            }
            tabPage4.Controls.Add(TP4_panel_ColorBar);
            TP4_panel_ColorBar.BorderStyle = BorderStyle.FixedSingle;
            TP4_panel_ColorBar.Visible = false;
            TP4_panel_ColorBar.Controls.AddRange(ColorBar);
            TP4_panel_ColorBar.Width = CB_With;
            TP4_panel_ColorBar.Top = TP4_panel_Resistor.Bottom - 1;
            Loop[0].Dock = DockStyle.Left;
            Loop[4].Dock = DockStyle.Right;
            TP4_comboBox_RUnit.SelectedIndex = 0;
            TP4_comboBox_Precision.Text = "±5%(J)";
            TP4_textBox_RValue.Text = "25";
            SetLoopCount(4);

        }

        private void SetLoopCount(int n)        //设置电阻的色环数
        {
            if (Ln == n)
                return;
            Ln = n;
            if (n == 5)
            {
                MyColorLoop CL = Loop[0];
                Loop[0] = Loop[1];
                Loop[1] = Loop[2];
                Loop[2] = CL;
                Loop[2].Visible = true;
                Loop[2].BackColor = Color.Black;
                int w = Color_Int(Loop[3].BackColor);
                if (w > -2)
                    w -= 1;
                Loop[3].BackColor = Int_Color(w);
                w = TP4_panel_Resistor.Width / 5;
                for (int i = 1; i < 4; i++)
                    Loop[i].Left = w * i;
            }
            else
            {
                Loop[2].Visible = false;
                MyColorLoop CL = Loop[2];
                Loop[2] = Loop[1];
                Loop[1] = Loop[0];
                Loop[0] = CL;
                int w = Color_Int(Loop[3].BackColor);
                if (w < 9)
                    w += 1;
                Loop[3].BackColor = Int_Color(w);
                w = TP4_panel_Resistor.Width / 4;
                for (int i = 1; i < 3; i++)
                    Loop[i + 1].Left = w * i;
            }
            Resistance_Loop();
        }

        private int Color_Int(Color c)          //把颜色转换为相应的数值
        {
            if (c == Color.Black)
                return 0;
            else if (c == Color.Brown)
                return 1;
            else if (c == Color.Red)
                return 2;
            else if (c == Color.Orange)
                return 3;
            else if (c == Color.Yellow)
                return 4;
            else if (c == Color.LimeGreen)
                return 5;
            else if (c == Color.Blue)
                return 6;
            else if (c == Color.Purple)
                return 7;
            else if (c == Color.Gray)
                return 8;
            else if (c == Color.White)
                return 9;
            else if (c == Color.Gold)
                return -1;
            else if (c == Color.Silver)
                return -2;
            else
                return -3;
        }

        private string Color_String(Color c)    //把颜色转换为相应的名称
        {
            if (c == Color.Black)
                return "黑色";
            else if (c == Color.Brown)
                return "棕色";
            else if (c == Color.Red)
                return "红色";
            else if (c == Color.Orange)
                return "橙色";
            else if (c == Color.Yellow)
                return "黄色";
            else if (c == Color.LimeGreen)
                return "绿色";
            else if (c == Color.Blue)
                return "蓝色";
            else if (c == Color.Purple)
                return "紫色";
            else if (c == Color.Gray)
                return "灰色";
            else if (c == Color.White)
                return "白色";
            else if (c == Color.Gold)
                return "金色";
            else if (c == Color.Silver)
                return "银色";
            else
                return "无色";
        }

        private string Color_Pricision(Color c) //把用颜色表示的精度转换为相应的文字
        {
            if (c == Color.Brown)
                return "±1%(T)";
            else if (c == Color.Red)
                return "±2%(G)";
            else if (c == Color.LimeGreen)
                return "±0.5%(D)";
            else if (c == Color.Blue)
                return "±0.25%(C)";
            else if (c == Color.Purple)
                return "±0.10%(B)";
            else if (c == Color.Gray)
                return "±0.05%(A)";
            else if (c == Color.Gold)
                return "±5%(J)";
            else if (c == Color.Silver)
                return "±10%(K)";
            else
                return "±20%(M)";
        }

        private bool Precision_Color(string s, out Color out1)      //把文字表示的精度转换为相应的颜色
        {
            out1 = Color.Brown;
            if (s == "±1%(T)")
                return true;
            else if (s == "±2%(G)")
                out1 = Color.Red;
            else if (s == "±0.5%(D)")
                out1 = Color.LimeGreen;
            else if (s == "±0.25%(C)")
                out1 = Color.Blue;
            else if (s == "±0.10%(B)")
                out1 = Color.Purple;
            else if (s == "±0.05%(A)")
                out1 = Color.Gray;
            else if (s == "±5%(J)")
                out1 = Color.Gold;
            else if (s == "±10%(K)")
                out1 = Color.Silver;
            else if (s == "±20%(M)")
                out1 = Color.Teal;
            else
                return false;
            return true;
        }

        private Color Int_Color(int a)          //把一个整数转换为相应的颜色
        {
            switch (a)
            {
                case -1:
                    return Color.Gold;
                case -2:
                    return Color.Silver;
                case 0:
                    return Color.Black;
                case 1:
                    return Color.Brown;
                case 2:
                    return Color.Red;
                case 3:
                    return Color.Orange;
                case 4:
                    return Color.Yellow;
                case 5:
                    return Color.LimeGreen;
                case 6:
                    return Color.Blue;
                case 7:
                    return Color.Purple;
                case 8:
                    return Color.Gray;
                case 9:
                    return Color.White;
            }
            if (a > 9)
                return Color.White;
            else
                return Color.Silver;
        }

        static public int Unit_Int(string s)    //把文本表示的单位转换为相应的Ω数
        {
            if (s == "KMΩ")
                return 1000000000;
            else if (s == "MΩ")
                return 1000000;
            else if (s == "KΩ")
                return 1000;
            else if (s == "Ω")
                return 1;
            else
                return -1;
        }

        private void Resistance_Loop()          //把文本表示的电阻值转换为相应的色环表示
        {
            if (TP4_textBox_RValue.Text.Length == 0)
            {
                TP4_label1.Text = "请输入电阻的大小";
                return;
            }
            if (TP4_comboBox_Precision.Text.Length == 0)
            {
                TP4_label1.Text = "请输入电阻精度";
                return;
            }
            if (TP4_comboBox_RUnit.Text.Length == 0)
            {
                TP4_label1.Text = "请输入电阻单位";
                return;
            }
            Color color;
            if (!Precision_Color(TP4_comboBox_Precision.Text, out color))
            {
                TP4_label1.Text = "精度不能是\"" + TP4_comboBox_Precision.Text + "\"";
                return;
            }
            Loop[4].BackColor = color;
            Double d;
            if (!Formula.Number_Compute2(TP4_textBox_RValue.Text, out d))
            {
                TP4_label1.Text = "您的输入有误";
                return;
            }
            if (d < 0)
            {
                TP4_label1.Text = "电阻不能为负数";
                return;
            }
            int a = Unit_Int(TP4_comboBox_RUnit.Text);
            if (a < 0)
            {
                TP4_label1.Text = "电阻值的单位输入有误";
                return;
            }
            int n = 0;
            if (d != 0)
            {
                d *= a;
                if (d < 0.01)
                {
                    TP4_label1.Text = "电阻值太小无法表示";
                    return;
                }
                int a1;
                if (Ln == 4)
                    a1 = 10;
                else
                    a1 = 100;
                if (d < a1)
                {
                    while (d < a1 && n > -2)
                    {
                        d *= 10;
                        n--;
                    }
                }
                else
                {
                    if (Ln == 4)
                        a1 = 99;
                    else
                        a1 = 999;
                    if (d > a1)
                    {
                        while (d > a1)
                        {
                            d /= 10;
                            n++;
                        }
                        if (n > 9)
                        {
                            TP4_label1.Text = "电阻值太大无法表示";
                            return;
                        }
                    }
                }
            }
            String s = d.ToString();
            if (s.Length > Ln - 2)
            {
                TP4_label1.Text = "有效数字位数过多 ";
                return;
            }
            for (int i = 2, j = s.Length - 1; i >= 5 - Ln; i--, j--)
            {
                if (j >= 0)
                    Loop[i].BackColor = Int_Color(s[j] - '0');
                else
                    Loop[i].BackColor = Color.Black;
            }
            Loop[3].BackColor = Int_Color(n);
            Loop_Resistance();
        }

        private void Loop_Resistance()          //把色环表示的电阻转换为相应的文本
        {
            double d;
            if (Ln == 5)
                d = Color_Int(Loop[0].BackColor) * 100 + Color_Int(Loop[1].BackColor) * 10 + Color_Int(Loop[2].BackColor);
            else
                d = Color_Int(Loop[1].BackColor) * 10 + Color_Int(Loop[2].BackColor);
            int n = Color_Int(Loop[3].BackColor);
            if (n > 0)
            {
                for (int i = 0; i < n; i++)
                    d *= 10;
            }
            else if (n < 0)
            {
                for (int i = n; i < 0; i++)
                    d *= 0.1;
            }
            string str;
            if (d >= 1E+9)
            {
                d /= 1E+9;
                str = "KMΩ";
            }
            else if (d >= 1E+6)
            {
                d /= 1E+6;
                str = "MΩ";
            }
            else if (d >= 1E+3)
            {
                d /= 1E+3;
                str = "KΩ";
            }
            else
                str = "Ω";
            TP4_label1.Text = "阻值：" + d + str + Color_Pricision(Loop[4].BackColor);
            if (IsLTR)
            {
                TP4_comboBox_Precision.Text = Color_Pricision(Loop[4].BackColor);
                TP4_comboBox_RUnit.Text = str;
                TP4_textBox_RValue.Text = d.ToString();
                TP4_textBox_RValue.Select(TP4_textBox_RValue.Text.Length, 0);
            }
        }

        void Loop_BackColorChanged(object sender, EventArgs e)  //当某色环的颜色改变时执行的操作
        {

            MyColorLoop l = (MyColorLoop)sender;
            l.Text = Color_String(l.BackColor);
            if (l.BackColor == Color.White || l.BackColor == Color.Yellow || l.BackColor == Color.Silver || l.BackColor == Color.Gold)
                l.ForeColor = Color.Black;
            else
                l.ForeColor = Color.White;
            if (IsLTR)
                Loop_Resistance();
        }

        private void ColorBar_Show()    //在当前色环下显示相应的选色条
        {
            if (CurrentLoop == null)
                return;
            IsLTR = true;
            TP4_panel_ColorBar.Left = TP4_panel_Resistor.Left + CurrentLoop.Left + 1;
            int[] a = CurrentLoop.color;
            TP4_panel_ColorBar.Height = a.Length * CB_Height;
            for (int i = 0; i < a.Length; i++)
            {
                ColorBar[a[i]].Top = CB_Height * i;
                ColorBar[a[i]].Visible = true;
            }
            TP4_panel_ColorBar.Visible = true;
        }

        private void ColorBar_Hide()    //隐藏选色条
        {
            IsLTR = false;
            for (int i = 0; i < 12; i++)
                ColorBar[i].Visible = false;
            TP4_panel_ColorBar.Visible = false;
        }

        void ColorBar_MouseEnter(object sender, EventArgs e)        //当鼠标在选色条中移动时执行的操作
        {
            if (CurrentLoop == null)
                return;
            CurrentLoop.BackColor = ((MyColorBar)sender).BackColor;
        }

        void ColorBar_MouseClick(object sender, MouseEventArgs e)   //当鼠标在选色条中单击时执行的操作
        {
            ColorBar_Hide();
        }

        void Loop_MouseEnter(object sender, EventArgs e)            //当鼠标进入某色环时执行的操作
        {
            if (!TP4_panel_ColorBar.Visible)
            {
                CurrentLoop = (MyColorLoop)sender;
                OldColor = CurrentLoop.BackColor;
                ColorBar_Show();
            }
        }

        private void TP4_panel_Resistor_MouseEnter(object sender, EventArgs e)  //当鼠标进入电阻体时执行的操作
        {
            if (TP4_panel_ColorBar.Visible)
            {
                CurrentLoop.BackColor = OldColor;
                ColorBar_Hide();
            }
        }

        private void tabPage4_MouseMove(object sender, MouseEventArgs e)        //当鼠标在空白区域移动时执行的操作
        {
            if (TP4_panel_ColorBar.Visible)
            {
                CurrentLoop.BackColor = OldColor;
                ColorBar_Hide();
            }
        }

        private void TP4_radioButton_Loop4_CheckedChanged(object sender, EventArgs e)   //改变电阻类型
        {
            if (TP4_radioButton_Loop4.Checked == true)
                SetLoopCount(4);
            else
                SetLoopCount(5);
        }

        private void TP4_button_RPConn_Click(object sender, EventArgs e)    //单击“电阻并联”时执行的操作
        {
            if (f5 == null || f5.IsDisposed)
            {
                try
                {
                    f5 = new Form5(Right, Top);
                    f5.button_Recognise.Click += new EventHandler(button_Recognise_Click);
                }
                catch (Exception)
                {
                    MessageBox.Show("内存空间不足");
                    return;
                }
                f5.Show();
            }
            else
                f5.Close();
        }

        void button_Recognise_Click(object sender, EventArgs e)             //在Form5中单击“识别”时执行的操作
        {
            ColorBar_Hide();
            Double d;
            TP4_comboBox_RUnit.SelectedIndex = f5.Compute(out d);
            String s1 = Math.Round(d, 2).ToString(), s2 = "";
            for (int i = 0, j = 0; i < s1.Length; i++)
            {
                if (s1[i] == '.')
                {
                    if (j >= Ln - 2)
                        break;
                    s2 += '.';
                }
                else
                {
                    if (j < Ln - 2)
                        s2 += s1[i];
                    else
                        s2 += '0';
                    j++;
                }
            }
            TP4_textBox_RValue.Text = s2;
        }

        private void TP4_button_Frequency_Click(object sender, EventArgs e)     //单击“频率计算”时执行的操作
        {
            if (f6 == null || f6.IsDisposed)
            {
                try
                {
                    f6 = new Form6(Right, Top);
                }
                catch (Exception)
                {
                    MessageBox.Show("内存空间不足");
                    return;
                }
                f6.Show();
            }
            else
                f6.Close();
        }

        private void TP4_textBox_RValue_TextChanged(object sender, EventArgs e) //当电阻值文本框的内容改变时执行的操作
        {
            if (IsLTR)
                return;
            Resistance_Loop();
        }

        #endregion

        #region tabPage5的代码

        Form7 f7;
        Form8 f8;
        Form9 f9;

        private void TabPage5()
        {

        }

        private void TP5_listView1_DoubleClick(object sender, EventArgs e)  //双击某项时执行的操作
        {
            if (TP5_listView1.SelectedItems[0].Index == 0)
            {
                if (f7 == null || f7.IsDisposed)
                {
                    try
                    {
                        f7 = new Form7(Right, Top);
                    }
                    catch (Exception)
                    {
                        MessageBox.Show("内存空间不足");
                        return;
                    }
                    f7.Show();
                }
            }
            else if (TP5_listView1.SelectedItems[0].Index == 1)
            {
                if (f8 == null || f8.IsDisposed)
                {
                    try
                    {
                        f8 = new Form8(Right, Top);
                    }
                    catch (Exception)
                    {
                        MessageBox.Show("内存空间不足");
                        return;
                    }
                    f8.Show();
                }
            }
            else if (TP5_listView1.SelectedItems[0].Index == 2)
            {
                if (f9 == null || f9.IsDisposed)
                {
                    try
                    {
                        f9 = new Form9(Right, Top);
                    }
                    catch (Exception)
                    {
                        MessageBox.Show("内存空间不足");
                        return;
                    }
                    f9.Show();
                }
            }
        }

        #endregion
    }

    #region 自定义的类

    public class Formula
    {
        private int angle_manner = 0;
        protected static String Error;
        protected String Fm;
        protected int Seek = 0;
        protected Stack S_Number;
        protected Stack S_Operator;
        private int a_A = 'a' - 'A';

        public static String ErrorText
        {
            get { return Error; }
            set { Error = value; }
        }

        public String formula
        {
            get { return Fm; }
            set { Fm = value; }
        }

        public Formula(String s1, int angle_manner)   //算式类的构造函数
        {
            Fm = s1;
            this.angle_manner = angle_manner;
            try
            {
                S_Number = new Stack(5);
                S_Operator = new Stack(5);
            }
            catch (Exception)
            {
                throw;
            }

        }

        public Formula() { }        //定义一个空的构造函数以便子类重载时覆盖

        static public bool Factorial(Double n, out Double out1)     //求n的阶乘
        {
            out1 = 1;
            if (n == 0)
                return true;
            ulong a = (ulong)n;
            if (n < 0 || n > a)
            {
                Error = "负数和浮点数不存在阶乘";
                return false;
            }
            while (a > 1)
                out1 *= (a--);
            return true;
        }

        static public Double Radian_Angle(Double d)//把幅度转换为角度
        {
            return d * 180 / Math.PI;
        }

        static public Double Angle_Radian(Double d)// 把角度转换为幅度
        {
            return d * Math.PI / 180;
        }

        static public bool D_F(Double di, int mode, out String out1)
        {
            out1 = "";
            if (di == (int)di)
            {
                out1 = di.ToString();
                return true;
            }
            Double d1 = Math.Floor(di);
            if (d1 != 0)
                di -= d1;
            if (di < 0)
                di = 0 - di;
            Double d2, a = 1, b = 2;
            while (b <= 1E+6)
            {
                d2 = Math.Round(a / b - di, 12);
                if (d2 == 0)
                {
                    if (mode == 0 || d1 == 0)
                        out1 = (a + d1 * b).ToString() + "/" + b;
                    else
                        out1 = d1.ToString() + "+" + a + "/" + b;
                    return true;
                }
                else if (d2 > 0)
                    b++;
                else
                    a++;
            }
            return false;
        }

        static public bool nAr(Double d1, Double d2, out Double out1)    //计算x个中选y个的排列数，并将结果赋给a
        {
            out1 = 0;
            if (d1 < 0 || d2 < 0)
            {
                Error = "负数不能进行排列组合运算";
                return false;
            }
            if (d1 > (int)d1 || d2 > (int)d2)
            {
                Error = "浮点数不能进行排列组合运算";
                return false;
            }
            if (d2 > d1)
            {
                Error = "不能从" + d1 + "个中选择" + d2 + "个作排列组合\n因为：" + d1 + "小于" + d2;
                return false;
            }
            if (d2 == 0)
                out1 = 1;
            else if (d1 == d2)
            {
                if (!Factorial(d1, out out1))
                    return false;
            }
            else
            {
                if (!Factorial(d1, out out1) || !Factorial(d1 - d2, out d1))
                    return false;
                out1 /= d1;
            }
            return true;
        }

        static public bool nCr(Double d1, Double d2, out Double out1)    //计算x个中选y个的组合数，并将结果赋给a
        {
            if (!nAr(d1, d2, out out1))
                return false;
            if (!Factorial(d2, out d2))
                return false;
            out1 /= d2;
            return true;
        }

        static public String Number_Adjust(Double d1, int number_manner, int precision)
        {
            if (d1 == 0)
                return "0";
            String s1 = "";
            if (number_manner == 0 && Math.Abs(d1) < 1E+15 && Math.Abs(d1) >= 1E-04)
            {
                if (precision > 15)
                    s1 = d1.ToString();
                else
                    s1 = Math.Round(d1, precision).ToString();
            }
            else
            {
                int n = 0;
                if (Math.Abs(d1) >= 10)
                {
                    while (Math.Abs(d1) >= 10)
                    {
                        d1 *= 0.1;
                        n++;
                    }
                }
                else if (Math.Abs(d1) < 1)
                {
                    while (Math.Abs(d1) < 1)
                    {
                        d1 *= 10;
                        n--;
                    }
                }
                if (precision > 15)
                    s1 = d1.ToString();
                else
                    s1 = Math.Round(d1, precision).ToString();
                s1 += "×10";
                if (n >= 0)
                    s1 += "^" + n;
                else
                    s1 += "^(" + n + ")";
            }
            return s1;
        }

        static public String Number_Adjust(Double d1)
        {
            return Number_Adjust(d1, 0, 15);
        }

        static public int PRI(Char s)        //优先级的定义
        {
            if (s == '+' || s == '-')
                return 1;
            else if (s == '*' || s == '/' || s == '×' || s == '÷' || s == '%')
                return 2;
            else if (s == 'A' || s == 'C')
                return 3;
            else if (s == 'N')
                return 4;
            else if (s == '^' || s == '√')
                return 5;
            else if (s == '²' || s == '³')
                return 6;
            else
                return 7;
        }

        static public bool IsNumber(Char a)//判断一个字符是否是数字字符
        {
            if (a >= '0' && a <= '9' || a == '.')
                return true;
            return false;
        }

        static public bool IsLetter(Char a)//判断一个字符是否是字母字符
        {
            if (a >= 'a' && a <= 'z' || a >= 'A' && a <= 'Z')
                return true;
            else
                return false;
        }

        static public bool IsEven(Double d)
        {
            String s = d.ToString();
            int n = s.IndexOf('E');
            if (n == -1)
                n = s.Length - 1;
            else
                n--;
            if (((int)s[n]) % 2 == 0)
                return true;
            else
                return false;
        }

        static public bool Root(Double index, Double d, out Double out1)
        {
            out1 = d;
            if (index == 0)
            {
                Error = "不能开0次方";
                return false;
            }
            else if (index == 1)
                return true;
            else if (d < 0)
            {
                if (IsEven(index))
                {
                    Error = "负数不能开偶数次方";
                    return false;
                }
                if (index == 2)
                    out1 = Math.Sqrt(0 - d);
                else
                    out1 = Math.Pow(0 - d, 1 / index);
                out1 = 0 - out1;
            }
            else if (index == 2)
                out1 = Math.Sqrt(d);
            else
                out1 = Math.Pow(d, 1 / index);
            return true;
        }

        static public bool Number_Compute(String si, int angle_manner, out Double out1)//计算一个用字符串表示的算式并用消息框给出出错信息
        {
            out1 = 0;
            if (si.Length == 0)
            {
                MessageBox.Show("请输入一个算式", "无法计算", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return false;
            }
            Formula f1;
            try
            {
                f1 = new Formula(si, angle_manner);
            }
            catch (Exception)
            {
                Error = "内存空间不足";
                return false;
            }
            if (!f1.Check_Bracket())
            {
                MessageBox.Show("算式非法：\n\n" + Error, "无法计算", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return false;
            }
            f1.Formula_Adjust();
            Error = null;
            if (!f1.Number_Compute(out out1))
            {
                if (Error == null)
                    MessageBox.Show("算式非法", "无法计算", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                else
                    MessageBox.Show("算式非法：\n\n" + Error, "无法计算", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return false;
            }
            return true;
        }

        static public bool IsESC(Char a)    //判断一个字符是不是转义字符
        {
            String s = "\'\"\\\0\a\b\f\n\r\t\v ";
            for (int i = 0; i < s.Length; i++)
                if (a == s[i])
                    return true;
            return false;
        }

        static public bool Number_Compute2(String si, out Double out1)  //计算一个算式不输出出错信息
        {
            out1 = 0;
            if (si.Length == 0)
                return false;
            Formula f1;
            try
            {
                f1 = new Formula(si, 0);
            }
            catch (Exception)
            {
                Error = "内存空间不足";
                return false;
            }
            if (!f1.Check_Bracket())
                return false;
            f1.Formula_Adjust();
            if (!f1.Number_Compute(out out1))
                return false;
            return true;
        }

        public bool Check_Bracket()     //检查括号是否配对
        {
            Stack stack;
            try
            {
                stack = new Stack(10);
            }
            catch (Exception)
            {
                Error = "内存空间不足";
                return false;
            }
            Object o1;
            for (int i = 0; i < Fm.Length; i++)
            {
                if (Fm[i] == '(' || Fm[i] == '[' || Fm[i] == '{')
                    stack.PushStack(Fm[i]);
                else if (Fm[i] == ')')
                {
                    if (!stack.PopStack(out o1))
                    {
                        Error = "存在多余的\")\"";
                        return false;
                    }
                    if ((Char)o1 != '(')
                    {
                        Error = "\")\"不配对";
                        return false;
                    }
                }
                else if (Fm[i] == ']')
                {
                    if (!stack.PopStack(out o1))
                    {
                        Error = "存在多余的\"]\"";
                        return false;
                    }
                    if ((Char)o1 != '[')
                    {
                        Error = "\"]\"不配对";
                        return false;
                    }

                }
                else if (Fm[i] == '}')
                {
                    if (!stack.PopStack(out o1))
                    {
                        Error = "存在多余的\"}\"";
                        return false;
                    }
                    if ((Char)o1 != '{')
                    {
                        Error = "\"}\"不配对";
                        return false;
                    }
                }
            }
            if (!stack.IsEmpty())
            {
                Error = "括号不配对";
                return false;
            }
            return true;
        }

        public void Formula_Adjust()     //把“{}”、“[]”转换为“()”
        {
            String s = Fm;
            Fm = "";
            for (int i = 0; i < s.Length; i++)
            {

                if (s[i] == '{' || s[i] == '[')
                    Fm += '(';
                else if (s[i] == '}' || s[i] == ']')
                    Fm += ')';
                else if (IsESC(s[i]))
                    continue;
                else
                    Fm += s[i];
            }
        }

        public String Cut_Bracket()    //从游标的当前位置开始提取一对括号中的内容并修改游标
        {
            String str = "";
            int n = 1;
            Seek++;
            for (; Seek < Fm.Length; Seek++)
            {
                if (Fm[Seek] == '(')
                    n++;
                else if (Fm[Seek] == ')')
                {
                    n--;
                    if (n <= 0)
                        break;
                }
                str += Fm[Seek];
            }
            Seek++;
            return str;
        }

        public bool Cut_Bracket(out Double out1, out Double out2)    //提取一对含有“，”的括号中的内容
        {
            String s1 = "";
            out1 = out2 = 0;
            Seek++;
            while (Seek < Fm.Length)
            {
                if (Fm[Seek] == '(')
                    s1 += "(" + Cut_Bracket() + ")";
                else if (Fm[Seek] == ',')
                {
                    Formula f1;
                    try
                    {
                        f1 = new Formula(s1, angle_manner);
                    }
                    catch (Exception)
                    {
                        Error = "内存空间不足";
                        return false;
                    }
                    if (!f1.Number_Compute(out out1))
                        return false;
                    s1 = "";
                    Seek++;
                }
                else if (Fm[Seek] == ')')
                {
                    Formula f1;
                    try
                    {
                        f1 = new Formula(s1, angle_manner);
                    }
                    catch (Exception)
                    {
                        Error = "内存空间不足";
                        return false;
                    }
                    if (!f1.Number_Compute(out out2))
                        return false;
                    break;
                }
                else
                    s1 += Fm[Seek++];
            }
            Seek++;
            return true;
        }

        public bool Cut_Double1(out String out1)//从游标的当前位置开始提取一个浮点数并给出出错信息
        {
            out1 = "";
            if (Fm[Seek] == '.')
            {
                Error = "小数点前无数字";
                return false;
            }
            for (; Seek < Fm.Length; Seek++)
            {
                if (Fm[Seek] >= '0' && Fm[Seek] <= '9')
                    out1 += Fm[Seek];
                else
                    break;
            }
            if (Seek < Fm.Length && Fm[Seek] == '.')
            {
                Seek++;
                out1 += '.';
                if (Seek >= Fm.Length || Fm[Seek] < '0' || Fm[Seek] > '9')
                {
                    Error = "小数点后无数字";
                    return false;
                }
                for (; Seek < Fm.Length; Seek++)
                {
                    if (Fm[Seek] >= '0' && Fm[Seek] <= '9')
                        out1 += Fm[Seek];
                    else
                        break;
                }
                if (Seek < Fm.Length && Fm[Seek] == '.')
                {
                    Error = "存在多余的小数点";
                    return false;
                }
            }
            return true;
        }

        public bool Cut_Double2(out String out1)
        {
            out1 = "";
            if (!Cut_Double1(out out1))
                return false;
            if (Seek < Fm.Length)
            {
                if (Fm[Seek] == 'E' || Fm[Seek] == 'e' && Seek + 1 < Fm.Length && Fm[Seek + 1] != '^' && Fm[Seek + 1] != '²' && Fm[Seek + 1] != '³')
                {
                    Seek++;
                    if (Seek >= Fm.Length)
                    {
                        Error = "\"E\"后无数字";
                        return false;
                    }
                    out1 += 'E';
                    if (Fm[Seek] == '-' || Fm[Seek] == '+')
                        out1 += Fm[Seek++];
                    if (Seek < Fm.Length && Fm[Seek] >= '0' && Fm[Seek] <= '9')
                    {
                        String ss;
                        if (!Cut_Double1(out ss))
                            return false;
                        out1 += ss;
                    }
                    else
                    {
                        Error = "\"E\"后无数字";
                        return false;
                    }
                }
            }
            return true;
        }
        private bool Cell_Compute(String O, Double d1, out Double out1)//运算符为字符串的单目运算
        {
            out1 = d1;
            if (O == "ln")
            {
                if (out1 <= 0)
                {
                    Error = "负数和0不存在自然对数";
                    return false;
                }
                out1 = Math.Log(out1, Math.E);
            }
            else if (O == "sqrt")
            {
                if (out1 < 0)
                {
                    Error = "负数不能开平方";
                    return false;
                }
                out1 = Math.Sqrt(out1);
            }
            else if (angle_manner == 0)
            {
                if (O == "sin")
                {
                    out1 = Math.IEEERemainder(d1, 180);
                    if (out1 == 0)
                        return true;
                    else if (Math.Abs(out1) == 180)
                        out1 = 0;
                    else if (out1 == 90)
                        out1 = 1;
                    else if (out1 == -90)
                        out1 = -1;
                    else
                        out1 = Math.Sin(Angle_Radian(out1));
                }
                else if ( O == "cos")
                {
                    out1 = Math.IEEERemainder(d1, 180);
                    if (Math.Abs(out1) == 90)
                        out1 = 0;
                    else if (out1 == 0)
                        out1 = 1;
                    else if (Math.Abs(out1) == 180)
                        out1 = -1;
                    else
                        out1 = Math.Cos(Angle_Radian(out1));
                }
                else if ( O == "tan")
                {
                    out1 = Math.IEEERemainder(d1, 90);
                    if (out1 == 0)
                        return true;
                    else if (Math.Abs(out1) == 90)
                    {
                        Error = "Tan(x)中的x不能等于n*180±90度";
                        return false;
                    }
                    else if (out1 == 45)
                        out1 = 1;
                    else if (out1 == -45)
                        out1 = -1;
                    else
                        out1 = Math.Tan(Angle_Radian(out1));
                }
                else if (O == "cot")
                {
                    out1 = Math.IEEERemainder(d1, 90);
                    if (out1 == 0)
                    {
                        Error = "Cot(x)中的x不能等于n*180度";
                        return false;
                    }
                    else if (out1 == 45)
                        out1 = 1;
                    else if (out1 == -45)
                        out1 = -1;
                    else if (Math.Abs(out1) == 90)
                        out1 = 0;
                    else
                        out1 = 1 / Math.Tan(Angle_Radian(out1));

                }
                else if (O == "arcsin")
                {
                    if (out1 == 0)
                        return true;
                    else if (Math.Abs(out1) > 1)
                    {
                        Error = "arcSin(a)中的a不能大于1";
                        return false;
                    }
                    else if (out1 == 1)
                        out1 = 90;
                    else if (out1 == -1)
                        out1 = -90;
                    else if (out1 == 0.5)
                        out1 = 30;
                    else if (out1 == -0.5)
                        out1 = -30;
                    else
                        out1 = Radian_Angle(Math.Asin(out1));
                }
                else if (O == "arccos")
                {
                    if (Math.Abs(out1) > 1)
                    {
                        Error = "arCos(a)中的a不能大于1";
                        return false;
                    }
                    else if (out1 == 1)
                        out1 = 0;
                    else if (out1 == -1)
                        out1 = 180;
                    else if (out1 == 0.5)
                        out1 = 60;
                    else if (out1 == -0.5)
                        out1 = 120;
                    else if (out1 == 0)
                        out1 = 90;
                    else
                        out1 = Radian_Angle(Math.Acos(out1));
                }
                else if (O == "arctan")
                {
                    if (out1 == 0)
                        return true;
                    else if (out1 == 1)
                        out1 = 45;
                    else if (out1 == -1)
                        out1 = -45;
                    else
                        out1 = Radian_Angle(Math.Atan(out1));
                }
                else if (O == "arccot")
                {
                    if (out1 == 0)
                        out1 = 90;
                    else if (out1 == 1)
                        out1 = 45;
                    else if (out1 == -1)
                        out1 = 135;
                    else
                        out1 = Radian_Angle(Math.Atan(1 / out1));
                }
                else
                {
                    Error = "运算符\"" + O + "\"非法使用";
                    return false;
                }
            }
            else
            {
                if (O == "sin")
                {
                    out1 = Math.IEEERemainder(d1, Math.PI);
                    if (out1 == 0)
                        return true;
                    else if (Math.Abs(out1) == Math.PI)
                        out1 = 0;
                    else if (out1 == Math.PI / 2)
                        out1 = 1;
                    else if (out1 == -Math.PI / 2)
                        out1 = -1;
                    else
                        out1 = Math.Sin(out1);
                }
                else if (O == "cos")
                {
                    out1 = Math.IEEERemainder(d1, Math.PI);
                    if (Math.Abs(out1) == Math.PI / 2)
                        out1 = 0;
                    else if (out1 == 0)
                        out1 = 1;
                    else if (Math.Abs(out1) == Math.PI)
                        out1 = -1;
                    else
                        out1 = Math.Cos(out1);
                }
                else if (O == "tan")
                {
                    out1 = Math.IEEERemainder(d1, Math.PI/2);
                    if (out1 == 0)
                        return true;
                    else if (Math.Abs(out1) == Math.PI / 2)
                    {
                        Error = "Tan(x)中的x不能等于n*180±90度";
                        return false;
                    }
                    else if (out1 == Math.PI / 4)
                        out1 = 1;
                    else if (out1 == -Math.PI / 4)
                        out1 = -1;
                    else
                        out1 = Math.Tan(out1);
                }
                else if (O == "cot")
                {
                    out1 = Math.IEEERemainder(d1, Math.PI/2);
                    if (out1 == 0)
                    {
                        Error = "Cot(x)中的x不能等于n*180度";
                        return false;
                    }
                    else if (out1 == Math.PI / 4)
                        out1 = 1;
                    else if (out1 == -Math.PI / 4)
                        out1 = -1;
                    else if (Math.Abs(out1) == Math.PI / 2)
                        out1 = 0;
                    else
                        out1 = 1 / Math.Tan(out1);

                }
                else if (O == "arcsin")
                {
                    if (out1 == 0)
                        return true;
                    else if (Math.Abs(out1) > 1)
                    {
                        Error = "arcSin(a)中的a不能大于1";
                        return false;
                    }
                    else if (out1 == 1)
                        out1 = Math.PI / 2;
                    else if (out1 == -1)
                        out1 = -Math.PI / 2;
                    else
                        out1 = Math.Asin(out1);
                }
                else if (O == "arccos")
                {
                    if (Math.Abs(out1) > 1)
                    {
                        Error = "arCos(a)中的a不能大于1";
                        return false;
                    }
                    else if (out1 == 1)
                        out1 = 0;
                    else if (out1 == -1)
                        out1 = Math.PI;
                    else if (out1 == 0)
                        out1 = Math.PI / 2;
                    else
                        out1 = Math.Acos(out1);
                }
                else if (O == "arctan")
                {
                    if (out1 == 0)
                        return true;
                    else if (out1 == 1)
                        out1 = Math.PI / 4;
                    else if (out1 == -1)
                        out1 = -Math.PI / 4;
                    else
                        out1 = Math.Atan(out1);
                }
                else if (O == "arccot")
                {
                    if (out1 == 0)
                        out1 = Math.PI / 2;
                    else if (out1 == 1)
                        out1 = Math.PI / 4;
                    else if (out1 == -1)
                        out1 = 3 * Math.PI / 4;
                    else
                        out1 = Math.Atan(1 / out1);
                }
                else
                {
                    Error = "运算符\"" + O + "\"非法使用";
                    return false;
                }
            }
            return true;
        }

        public bool Number_Compute(out Double out1)//计算自己的算式，不检查和改变括号，以便提高递归的效率
        {
            out1 = 0;
            if (Fm.Length <= 0)
            {
                Error = "括号中无算式";
                return false;
            }
            if (Fm[Seek] == '-' || Fm[Seek] == '+')
                S_Number.PushStack(0.0);
            while (Seek < Fm.Length)
            {
                if (IsNumber(Fm[Seek]))
                {
                    String s;
                    if (!Cut_Double2(out s))
                        return false;
                    S_Number.PushStack(Double.Parse(s));
                }
                else if (IsOperator(Seek))
                {
                    if (Fm[Seek] == '√')
                        if (Seek <= 0 || !(Fm[Seek - 1] == ')' || IsNumber(Fm[Seek - 1]) || IsLetter(Fm[Seek - 1])))
                            S_Number.PushStack(2.0);
                    if (!Operator_Compute(Fm[Seek++]))
                        return false;
                }
                else if (IsLetter(Fm[Seek]))
                {
                    if (Seek > 0)
                        if (IsNumber(Fm[Seek - 1]) || IsLetter(Fm[Seek - 1]) || Fm[Seek - 1] == '²' || Fm[Seek - 1] == '³')
                        {
                            if (!Operator_Compute('N'))
                                return false;
                        }
                    String s;
                    if (Cut_KeyWord(out s))
                    {
                        Double d1 = 1;
                        if (s == "e" || s == "pi")
                        {
                            if (s == "e")
                                d1 = Math.E;
                            else
                                d1 = Math.PI;
                        }
                        else
                        {
                            Stack S_String;
                            try
                            {
                                S_String = new Stack(5);
                            }
                            catch (Exception)
                            {
                                Error = "内存空间不足";
                                return false;
                            }
                            S_String.PushStack(s);
                            while (Seek < Fm.Length)
                            {
                                if (IsOperator(Seek) || Fm[Seek] == '!')
                                {
                                    Error = "存在多余的运算符\"" + Fm[Seek] + "\"";
                                    return false;
                                }
                                else if (!IsLetter(Fm[Seek]))
                                    break;
                                if (!Cut_KeyWord(out s))
                                    return false;
                                if (s == "e" || s == "pi")
                                    break;
                                S_String.PushStack(s);
                            }
                            bool HaveData = false;
                            if (Seek < Fm.Length && Fm[Seek] == '(')
                            {
                                Object O1;
                                S_String.GetTopData(out O1);
                                s = (String)O1;
                                if (s == "log" || s == "root")
                                {
                                    HaveData = true;
                                    S_String.PopStack(out O1);
                                    Double d2, d3;
                                    if (!Cut_Bracket(out  d2, out d3))
                                        return false;
                                    if (s == "log")
                                    {
                                        d1 = Math.Log(d3, d2);
                                        if (Double.IsNaN(d1))
                                        {
                                            Error = "在求对数时遇到语法错误";
                                            return false;
                                        }
                                    }
                                    else if (!Root(d2, d3, out d1))
                                        return false;
                                }
                            }
                            if (s != "e" && s != "pi")
                                s = "";
                            while (Seek < Fm.Length)
                            {
                                if (Fm[Seek] == '(')
                                    s += "(" + Cut_Bracket() + ")";
                                else if (Fm[Seek] == 'E')
                                {
                                    s += Fm[Seek++];
                                    if (Seek < Fm.Length)
                                        s += Fm[Seek++];
                                    else
                                    {
                                        Error = "\"E\"后无数字";
                                        return false;
                                    }
                                }
                                else if (Fm[Seek] == '√' || PRI(Fm[Seek]) < PRI('^'))
                                    break;
                                else if (IsLetter(Fm[Seek]))
                                {
                                    if (Fm[Seek] == 'e')
                                        s += Fm[Seek++];
                                    else if (Fm[Seek] == 'P' || Fm[Seek] == 'p')
                                    {
                                        s += Fm[Seek++];
                                        if (Seek >= Fm.Length)
                                        {
                                            Error = "存在非法字符\"" + Fm[Seek - 1] + "\"";
                                            return false;
                                        }
                                        else if (Fm[Seek] != 'i' && Fm[Seek] != 'I')
                                        {
                                            Error = "存在非法字串\"" + Fm[Seek - 1] + Fm[Seek] + "\"";
                                            return false;
                                        }
                                        else
                                            s += Fm[Seek++];
                                    }
                                    else
                                        break;
                                }
                                else
                                    s += Fm[Seek++];
                            }
                            if (s.Length == 0)
                            {
                                if (!HaveData)
                                {
                                    Error = "缺少操作数";
                                    return false;
                                }
                            }
                            else
                            {
                                Formula f = new Formula(s, angle_manner);
                                if (HaveData)
                                    f.S_Number.PushStack(d1);
                                if (!f.Number_Compute(out d1))
                                    return false;
                            }
                            Object O2;
                            while (!S_String.IsEmpty())
                            {
                                S_String.PopStack(out O2);
                                if (!Cell_Compute((String)O2, d1, out d1))
                                    return false;
                            }
                        }
                        S_Number.PushStack(d1);
                    }
                    else
                    {
                        Error = "存在非法字串：\"" + s + "\"";
                        return false;
                    }
                }
                else if (Fm[Seek] == '(')
                {
                    Double d1 = 1;
                    if (Seek > 0 && !IsOperator(Seek - 1))
                    {
                        if (IsNumber(Fm[Seek - 1]) || IsLetter(Fm[Seek - 1]) || Fm[Seek - 1] == ')')
                        {
                            if (!Operator_Compute('*'))
                                return false;
                        }
                    }
                    Formula f1;
                    try
                    {
                        f1 = new Formula(Cut_Bracket(), angle_manner);
                    }
                    catch (Exception)
                    {
                        Error = "内存空间不足";
                        return false;
                    }
                    if (!f1.Number_Compute(out d1))
                        return false;
                    S_Number.PushStack(d1);
                }
                else if (Fm[Seek] == '!')
                {
                    Object o1;
                    if (!S_Number.PopStack(out o1))
                    {
                        Error = "存在多余运算符\"!\"";
                        return false;
                    }
                    Double a1;
                    if (!Factorial((Double)o1, out a1))
                        return false;
                    S_Number.PushStack(a1);
                    Seek++;
                }
                else if (IsESC(Fm[Seek]))
                    Seek++;
                else
                {
                    Error = "存在非法字符：\"" + Fm[Seek] + "\"";
                    return false;
                }
            }
            Object data, sy;
            if (!S_Number.PopStack(out data))
            {
                Error = "存在多余的运算符";
                return false;
            }
            out1 = (Double)data;
            while (S_Operator.PopStack(out sy))
            {
                if (!S_Number.PopStack(out data))
                {
                    Error = "存在多余的运算符\"" + (Char)sy + "\"";
                    return false;
                }
                if (!Cell_Compute((Char)sy, (Double)data, out1, out out1))
                    return false;
            }
            if (!S_Number.IsEmpty())
            {
                Error = "缺少运算符";
                return false;
            }
            if (Double.IsNaN(out1))
                return false;
            if (Double.IsInfinity(out1))
            {
                Error = "数值太大，已溢出";
                out1 = 0;
                return false;
            }
            return true;
        }

        private bool IsOperator(int index)//判断游标当前位置的字符是否是一个运算符
        {
            if (Fm[index] == 'C')
            {
                index++;
                if (index < Fm.Length && Fm[index] != 'o')
                    return true;
                else
                    return false;
            }
            String str = "+-*×/÷%^A√²³";
            for (int i = 0; i < str.Length; i++)
                if (Fm[index] == str[i])
                    return true;
            return false;
        }

        private bool Operator_Compute(Char O)//当遇到运算符时执行的操作
        {
            Object O1;
            if (!S_Operator.GetTopData(out O1) || PRI((Char)O1) < PRI(O))
                S_Operator.PushStack(O);
            else
            {
                Object d1;
                Double d2;
                if (!S_Number.PopStack(out d1))
                {
                    Error = "存在多余运算符";
                    return false;
                }
                d2 = (Double)d1;
                while (S_Operator.GetTopData(out O1) && PRI((Char)O1) >= PRI(O))
                {
                    S_Operator.PopStack(out O1);
                    if (!S_Number.PopStack(out d1))
                    {
                        Error = "存在多余运算符\"" + (Char)O1 + "\"";
                        return false;
                    }
                    if (!Cell_Compute((Char)O1, (Double)d1, d2, out d2))
                        return false;
                }
                S_Number.PushStack(d2);
                S_Operator.PushStack(O);
            }
            if (O == '²')
                S_Number.PushStack(2.0);
            else if (O == '³')
                S_Number.PushStack(3.0);
            return true;
        }

        private bool Cut_KeyWord(out String out1)//从游标的当前位置开始提取一个关键字，并给出出错信息
        {

            out1 = "";
            for (; Seek < Fm.Length; Seek++)
            {
                if (IsLetter(Fm[Seek]))
                {
                    if (Fm[Seek] < 'a')
                        out1 += (Char)(Fm[Seek] + a_A);
                    else
                        out1 += Fm[Seek];
                }
                else
                    break;
                if (IsWordOperator(out1))
                {
                    Seek++;
                    return true;
                }
            }
            Error = "存在非法字串：\"" + out1 + "\"";
            return false;
        }

        private bool IsWordOperator(String si)//判断一个字符串是否是关键字
        {
            String[] str = { };
            try
            {
                switch (si.Length)
                {
                    case 1:
                        str = new String[] { "e" };
                        break;
                    case 2:
                        str = new String[] { "ln", "pi" };
                        break;
                    case 3:
                        str = new String[] { "log", "sin", "cos", "tan", "cot", };
                        break;
                    case 4:
                        str = new String[] { "root", "sqrt" };
                        break;
                    case 6:
                        str = new String[] { "arcsin", "arccos", "arctan", "arccot" };
                        break;
                }
            }
            catch (Exception)
            {
                Error = "内存空间不足";
                return false;
            }
            for (int i = 0; i < str.Length; i++)
                if (str[i] == si)
                    return true;
            return false;
        }

        private bool Cell_Compute(Char O, Double d1, Double d2, out Double out1)//双目运算
        {
            out1 = d1;
            if (O == '+')
                out1 += d2;
            else if (O == '-')
                out1 -= d2;
            else if (O == '*' || O == '×' || O == 'N')
                out1 *= d2;
            else if (O == '/' || O == '÷')
            {
                if (d2 == 0)
                {
                    Error = "0不能做除数";
                    return false;
                }
                out1 /= d2;
            }
            else if (O == '%')
            {
                if (d2 == 0)
                {
                    Error = "0不能做除数";
                    return false;
                }
                out1 = Math.IEEERemainder(d1, d2);
                if (out1 < 0)
                    out1 += d2;
            }
            else if (O == '^' || O == '²' || O == '³')
            {
                if (d2 == 0)
                    out1 = 1;
                else if (d1 < 0)
                {
                    out1 = Math.Pow(0 - d1, d2);
                    if (!IsEven(d2))
                        out1 = 0 - out1;
                }
                else
                    out1 = Math.Pow(out1, d2);
            }
            else if (O == 'A')
            {
                Double a1;
                if (!nAr(d1, d2, out a1))
                    return false;
                out1 = a1;
            }
            else if (O == 'C')
            {
                Double a1;
                if (!nCr(d1, d2, out a1))
                    return false;
                out1 = a1;
            }
            else if (O == '√')
            {
                if (!Root(d1, d2, out out1))
                    return false;
            }
            else
            {
                Error = "运算符\"" + O + "\"非法使用";
                return false;
            }
            return true;
        }
    }

    public class MultinomialA   //存储多项式中的一个字母
    {
        public Char letter;     //字母
        public Double index;    //指数

        public MultinomialA(Char letter, Double index)  //构造函数
        {
            this.letter = letter;
            this.index = index;
        }

        public MultinomialA(Char letter)    //构造函数
        {
            this.letter = letter;
            this.index = 1;
        }

        public MultinomialA(MultinomialA a) //复制构造函数
        {
            letter = a.letter;
            index = a.index;
        }

        static public int Compare(MultinomialA a, MultinomialA b)   //比较两个A类结点的大小（字母升序，指数降序）
        {
            if (a.letter != b.letter)
                return a.letter - b.letter;
            else
                return (int)(b.index - a.index);
        }
    }

    public class MultinomialB       //存储一个不含运算符的多项式
    {
        private int BM = 5;         //标准容量
        private int M;              //当前容量
        public int count = 0;       //结点数目
        public Double coefficient;  //系数
        public MultinomialA[] Buf;  //存储空间

        public MultinomialB(Double coefficient) //构造函数
        {
            M = BM;
            try
            {
                Buf = new MultinomialA[M];
            }
            catch (Exception)
            {
                throw;
            }
            this.coefficient = coefficient;
        }

        public MultinomialB(MultinomialB a)     //复制构造函数
        {
            BM = a.BM;
            M = a.M;
            count = a.count;
            coefficient = a.coefficient;
            try
            {
                Buf = new MultinomialA[M];
                for (int i = 0; i < count; i++)
                    Buf[i] = new MultinomialA(a.Buf[i]);
            }
            catch (Exception)
            {
                throw;
            }
        }

        private void Enlarge()  //扩大存储空间
        {
            MultinomialA[] old = Buf;
            try
            {
                Buf = new MultinomialA[M * 2];
            }
            catch (Exception)
            {
                throw;
            }
            for (int i = 0; i < M; i++)
                Buf[i] = old[i];
            M *= 2;
        }

        private void Reduce()   //缩减存储空间
        {
            if (M <= BM)
                return;
            MultinomialA[] old = Buf;
            M /= 2;
            try
            {
                Buf = new MultinomialA[M];
            }
            catch (Exception)
            {
                throw;
            }
            for (int i = 0; i < M; i++)
                Buf[i] = old[i];
        }

        public void Insert(MultinomialA a)  //插入一个A类结点（实为多项式的乘法运算）
        {
            if (a.index == 0)
                return;
            int n = 0;
            while (n < count)
            {
                if (a.letter <= Buf[n].letter)
                    break;
                n++;
            }
            if (n < count && a.letter == Buf[n].letter)
            {
                Buf[n].index += a.index;
                if (Buf[n].index == 0)
                    Remove(n);
            }
            else
            {
                if (count >= M)
                    Enlarge();
                if (n < count)
                {
                    for (int i = count; i > n; i--)
                        Buf[i] = Buf[i - 1];
                }
                try
                {
                    Buf[n] = new MultinomialA(a);
                }
                catch (Exception)
                {
                    throw;
                }
                count++;
            }
        }

        static public int Compare(MultinomialB a, MultinomialB b)       //比较两个B类结点的大小（系数部分不算）
        {
            int i = 0, n;
            while (i < a.count && i < b.count)
            {
                n = MultinomialA.Compare(a.Buf[i], b.Buf[i]);
                if (n != 0)
                    return n;
                i++;
            }
            return a.count - b.count;
        }

        public bool Remove(int index)       //删除一个结点
        {
            if (index < 0 || index >= count)
                return false;
            for (int i = index + 1; i < count; i++)
                Buf[i - 1] = Buf[i];
            count--;
            Buf[count] = null;
            if (count < M / 4)
                Reduce();
            return true;
        }

        static public MultinomialB MUL(MultinomialB a, MultinomialB b)  //B类结点的乘法运算
        {
            try
            {
                if (a.coefficient == 0 || b.coefficient == 0)
                    return new MultinomialB(0);
            }
            catch (Exception)
            {
                throw;
            }
            MultinomialB c;
            try
            {
                c = new MultinomialB(a);
            }
            catch (Exception)
            {
                throw;
            }
            c.coefficient *= b.coefficient;
            for (int i = 0; i < b.count; i++)
                c.Insert(b.Buf[i]);
            return c;
        }

        public bool IsZero()                //判断一个B类结点是否为0
        {
            if (coefficient == 0)
                return true;
            return false;
        }

        public bool IsMultinomial()         //判断一个B类结点是否是多项式
        {
            if (count == 0)
                return false;
            return true;
        }

        public override String ToString()   //把一个B类结点转换为字符串
        {
            if (coefficient == 0)
                return "0";
            if (count == 0)
                return Formula.Number_Adjust(coefficient);
            String s = "";
            if (coefficient == -1)
                s += '-';
            else if (coefficient != 1)
                s += coefficient.ToString();
            for (int i = 0; i < count; i++)
            {
                if (Buf[i].index == 0)
                    continue;
                else if (Buf[i].index == 1)
                    s += Buf[i].letter;
                else
                {
                    if (Buf[i].index > 0)
                        s += Buf[i].letter + "^" + Buf[i].index;
                    else
                        s += Buf[i].letter + "^(" + Buf[i].index + ")";
                    if (i < count - 1)
                        s += "*";
                }
            }
            return s;
        }

        public int Indexof(Char a)          //查找字母为a的A类结点的索引
        {
            for (int i = 0; i < count; i++)
                if (Buf[i].letter == a)
                    return i;
            return -1;
        }
    }

    public class MultinomialC : Formula     //存储一个多项式
    {
        private int BM = 5;         //标准容量
        private int M;              //当前容量
        public int count = 0;       //结点数
        public MultinomialB[] Buf;  //存储空间

        public MultinomialC()   //构造函数
        {
            M = BM;
            try
            {
                Buf = new MultinomialB[M];
            }
            catch (Exception)
            {
                throw;
            }
        }

        public MultinomialC(String s1)      //构造函数
        {
            Fm = s1;
            try
            {
                S_Number = new Stack(5);
                S_Operator = new Stack(5);
            }
            catch (Exception)
            {
                throw;
            }
        }

        public MultinomialC(MultinomialC a) //复制构造函数
        {
            BM = a.BM;
            M = a.M;
            count = a.count;
            try
            {
                Buf = new MultinomialB[M];
                for (int i = 0; i < count; i++)
                    Buf[i] = new MultinomialB(a.Buf[i]);
            }
            catch (Exception)
            {
                throw;
            }
        }

        public MultinomialC(MultinomialB a) //构造函数
        {
            M = BM;
            try
            {
                Buf = new MultinomialB[M];
            }
            catch (Exception)
            {
                throw;
            }
            Insert(a);
        }

        private void Enlarge()  //扩大存储空间
        {
            MultinomialB[] old = Buf;
            try
            {
                Buf = new MultinomialB[M * 2];
            }
            catch (Exception)
            {
                throw;
            }
            for (int i = 0; i < M; i++)
                Buf[i] = old[i];
            M *= 2;
        }

        private void Reduce()   //缩减存储空间
        {
            if (M <= BM)
                return;
            MultinomialB[] old = Buf;
            M /= 2;
            try
            {
                Buf = new MultinomialB[M];
            }
            catch (Exception)
            {
                throw;
            }
            for (int i = 0; i < M; i++)
                Buf[i] = old[i];
        }

        public void Insert(MultinomialB a)  //插入一个B类结点（实为多项式的加法运算）
        {
            if (a.IsZero())
                return;
            int n = 0;
            while (n < count)
            {
                if (MultinomialB.Compare(a, Buf[n]) <= 0)
                    break;
                n++;
            }
            if (n < count && MultinomialB.Compare(a, Buf[n]) == 0)
            {
                Buf[n].coefficient += a.coefficient;
                if (Buf[n].coefficient == 0)
                    Remove(n);
            }
            else
            {
                if (count >= M)
                    Enlarge();
                if (n < count)
                {
                    for (int i = count; i > n; i--)
                        Buf[i] = Buf[i - 1];
                }
                try
                {
                    Buf[n] = new MultinomialB(a);
                }
                catch (Exception)
                {
                    throw;
                }
                count++;
            }
        }

        public bool IsNumber()              //判断一个多项式是否是一个数值
        {
            if (count > 1)
                return false;
            if (Buf[0].count > 0)
                return false;
            return true;
        }

        public bool Remove(int index)       //删除一个B类结点
        {
            if (index < 0 || index >= count)
                return false;
            for (int i = index + 1; i < count; i++)
                Buf[i - 1] = Buf[i];
            count--;
            Buf[count] = null;
            return true;
        }

        static public MultinomialC Add(MultinomialC a, MultinomialC b)  //两个多项式的加法
        {
            MultinomialC c;
            try
            {
                c = new MultinomialC(a);
            }
            catch (Exception)
            {
                throw;
            }
            for (int i = 0; i < b.count; i++)
                c.Insert(b.Buf[i]);
            return c;
        }

        static private MultinomialC Sub(MultinomialC a, MultinomialC b) //两个多项式的减法
        {
            MultinomialC c;
            MultinomialC d;
            try
            {
                c = new MultinomialC(a);
                d = new MultinomialC(b);
            }
            catch (Exception)
            {
                throw;
            }
            for (int i = 0; i < d.count; i++)
            {
                d.Buf[i].coefficient *= -1;
                c.Insert(d.Buf[i]);
            }
            return c;
        }

        static public MultinomialC MUL(MultinomialC a, MultinomialC b)  //两个多项式的乘法
        {
            MultinomialC c;
            try
            {
                c = new MultinomialC();
            }
            catch (Exception)
            {
                throw;
            }
            for (int i = 0; i < a.count; i++)
                for (int j = 0; j < b.count; j++)
                    c.Insert(MultinomialB.MUL(a.Buf[i], b.Buf[j]));
            return c;
        }

        static public bool IncludeLetter(String si) //判断一个字符串是否包含字母
        {
            for (int i = 0; i < si.Length; i++)
                if (IsLetter(si[i]))
                    return true;
            return false;
        }

        private bool Degree(out Double out1)        //计算游标当前位置处的指数
        {
            out1 = 1;
            Seek++;
            if (Seek >= Fm.Length)
            {
                Error = "\"^\"后面没有数字";
                return false;
            }
            while (Seek < Fm.Length)
            {
                if (Fm[Seek] != '^' && IsOperator(Fm[Seek]) || IsLetter(Fm[Seek]))
                    break;
                else if (Fm[Seek] == '^')
                    Seek++;
                else if (IsNumber(Fm[Seek]))
                {
                    String s;
                    if (!Cut_Double2(out s))
                        return false;
                    out1 *= Double.Parse(s);
                }
                else if (Fm[Seek] == '(')
                {
                    String s = Cut_Bracket();
                    Double d;
                    Formula f1;
                    try
                    {
                        f1 = new Formula(s, 0);
                    }
                    catch (Exception)
                    {
                        Error = "内存空间不足";
                        return false;
                    }
                    if (f1.Number_Compute(out d))
                        out1 *= d;
                    else
                    {
                        if (IncludeLetter(s))
                            Error = "多项式不能作为指数";
                        return false;
                    }
                }
                else if (IsESC(Fm[Seek]))
                    Seek++;
                else
                {
                    Error = "存在非法字符：\"" + Fm[Seek] + "\"";
                    return false;
                }
            }
            return true;
        }

        private bool ConstructMultinomialB(out MultinomialB out1)                   //从游标当前位置开始构造一个B类结点
        {
            out1 = null;
            try
            {
                out1 = new MultinomialB(1);
            }
            catch (Exception)
            {
                Error = "内存空间不足";
                return false;
            }
            while (Seek < Fm.Length)
            {
                if (Fm[Seek] != '^' && IsOperator(Fm[Seek]) || Fm[Seek] == '(')
                    break;
                else if (IsLetter(Fm[Seek]))
                {
                    MultinomialA a;
                    try
                    {
                        a = new MultinomialA(Fm[Seek++]);
                    }
                    catch (Exception)
                    {
                        Error = "内存空间不足";
                        return false;
                    }
                    if (Seek < Fm.Length && Fm[Seek] == '^')
                    {
                        Double d;
                        if (!Degree(out d))
                            return false;
                        a.index = d;
                    }
                    out1.Insert(a);
                }
                else if (IsESC(Fm[Seek]))
                    Seek++;
                else
                {
                    if (IsNumber(Fm[Seek]))
                        Error = "字母后面不能直接跟数字";
                    else
                        Error = "存在非法字符：\"" + Fm[Seek] + "\"";
                    return false;
                }
            }
            return true;
        }

        private bool Div(MultinomialC a, MultinomialC b, out MultinomialC out1)     //多项式除以一个数值
        {
            out1 = null;
            try
            {
                out1 = new MultinomialC(a);
            }
            catch (Exception)
            {
                Error = "内存空间不足";
                return false;
            }
            if (b.count == 0)
            {
                Error = "0不能做除数";
                return false;
            }
            if (!b.IsNumber())
            {
                Error = "多项式不能做除数";
                return false;
            }
            if (out1.count == 0)
                return true;
            if (out1.IsNumber())
            {
                out1.Buf[0].coefficient /= b.Buf[0].coefficient;
                return true;
            }
            Double n = (int)b.Buf[0].coefficient;
            for (int i = 0; i < out1.count; i++)
                out1.Buf[i].coefficient /= n;
            return true;
        }

        private bool IsOperator(Char a)             //判断一个字符是否是运算符
        {
            String s = "+-*/^";
            for (int i = 0; i < s.Length; i++)
                if (a == s[i])
                    return true;
            return false;
        }

        private bool Cell_Compute(Char O, MultinomialC a, MultinomialC b, out MultinomialC out1)        //多项式的单目运算
        {
            out1 = null;
            switch (O)
            {
                case '+':
                    out1 = Add(a, b);
                    break;
                case '-':
                    out1 = Sub(a, b);
                    break;
                case '*':
                    out1 = MUL(a, b);
                    break;
                case 'N':
                    out1 = MUL(a, b);
                    break;
                case '^':
                    if (!Pow(a, b, out out1))
                        return false;
                    break;
                case '/':
                    if (!Div(a, b, out out1))
                        return false;
                    break;
                default:
                    Error = "存在非法的运算符：" + O + "";
                    return false;
            }
            return true;
        }

        private bool Operator_Compute(Char a)       //当游标当前位置是一个运算符时执行的操作
        {
            Object O;
            if (S_Operator.GetTopData(out O) && PRI((Char)O) >= PRI(a))
            {
                Object d1;
                MultinomialC d2;
                if (!S_Number.PopStack(out d1))
                {
                    Error = "存在多余运算符";
                    return false;
                }
                d2 = (MultinomialC)d1;
                while (S_Operator.GetTopData(out O) && PRI((Char)O) >= PRI(a))
                {
                    S_Operator.PopStack(out O);
                    if (!S_Number.PopStack(out d1))
                    {
                        Error = "存在多余运算符";
                        return false;
                    }
                    if (!Cell_Compute((Char)O, (MultinomialC)d1, d2, out d2))
                        return false;
                }
                S_Number.PushStack(d2);

            }
            S_Operator.PushStack(a);
            return true;
        }

        private void Clear()                         //将一个多项式值零
        {
            M = BM;
            count = 0;
            try
            {
                Buf = new MultinomialB[M];
            }
            catch (Exception)
            {
                throw;
            }
        }

        private bool Pow(MultinomialC a, MultinomialC b, out MultinomialC out1)       //多项式的指数运算
        {
            out1 = null;
            if (b.count == 0)
            {
                if (a.count == 0)
                {
                    MessageBox.Show("不能计算0的0次幂");
                    return false;
                }
                try
                {
                    out1 = new MultinomialC(new MultinomialB(1));
                }
                catch (Exception)
                {
                    Error = "内存空间不足";
                    return false;
                }
                return true;
            }
            if (!b.IsNumber())
            {
                Error = "指数不能是多项式";
                return false;
            }
            if (a.count == 0)
            {
                try
                {
                    out1 = new MultinomialC();
                }
                catch (Exception)
                {
                    Error = "内存空间不足";
                    return false;
                }
                return true;
            }
            if (a.IsNumber())
            {
                try
                {
                    out1 = new MultinomialC(new MultinomialB(Math.Pow(a.Buf[0].coefficient, b.Buf[0].coefficient)));
                }
                catch (Exception)
                {
                    Error = "内存空间不足";
                    return false;
                }
                if (Double.IsNaN(out1.Buf[0].coefficient))
                    return false;
                return true;
            }
            int n = (int)b.Buf[0].coefficient;
            if (n < 0)
            {
                Error = "多项式不能计算负数的幂";
                return false;
            }
            try
            {
                out1 = new MultinomialC(a);
            }
            catch (Exception)
            {
                Error = "内存空间不足";
                return false;
            }
            for (int i = 1; i < n; i++)
                out1 = MUL(out1, a);
            return true;
        }

        public bool Multinomial_Compute(out MultinomialC out1)                        //多项式的综合运算
        {
            out1 = null;
            if (Fm.Length == 0)
            {
                Error = "括号中无内容";
                return false;
            }
            if (Fm[0] == '-')
            {
                try
                {
                    S_Number.PushStack(new MultinomialC());
                }
                catch (Exception)
                {
                    Error = "内存空间不足";
                    return false;
                }
            }
            while (Seek < Fm.Length)
            {
                if (IsOperator(Fm[Seek]))
                {
                    if (!Operator_Compute(Fm[Seek++]))
                        return false;
                }
                else if (IsNumber(Fm[Seek]) || IsLetter(Fm[Seek]))
                {
                    if (Seek > 0 && Fm[Seek - 1] == ')')
                        if (!Operator_Compute('*'))
                            return false;
                    Double d = 1;
                    if (IsNumber(Fm[Seek]))
                    {
                        String s;
                        if (!Cut_Double2(out s))
                            return false;
                        d = Double.Parse(s);
                    }
                    if (Seek < Fm.Length && IsLetter(Fm[Seek]))
                    {
                        MultinomialB b;
                        if (!ConstructMultinomialB(out b))
                            return false;
                        b.coefficient = d;
                        try
                        {
                            S_Number.PushStack(new MultinomialC(b));
                        }
                        catch (Exception)
                        {
                            Error = "内存空间不足";
                            return false;
                        }
                    }
                    else
                    {
                        try
                        {
                            S_Number.PushStack(new MultinomialC(new MultinomialB(d)));
                        }
                        catch (Exception)
                        {
                            Error = "内存空间不足";
                            return false;
                        }
                    }
                }
                else if (Fm[Seek] == '(')
                {
                    if (Seek > 0)
                    {
                        if (IsNumber(Fm[Seek - 1]) || IsLetter(Fm[Seek - 1]) || Fm[Seek - 1] == ')')
                        {
                            if (!Operator_Compute('*'))
                                return false;
                        }
                    }
                    String s = Cut_Bracket();
                    Formula f;
                    Double d;
                    try
                    {
                        f = new Formula(s, 0);
                        if (f.Number_Compute(out d))
                            S_Number.PushStack(new MultinomialC(new MultinomialB(d)));
                        else
                        {
                            MultinomialC c = new MultinomialC(s);
                            if (!c.Multinomial_Compute(out c))
                                return false;
                            S_Number.PushStack(c);
                        }
                    }
                    catch (Exception)
                    {
                        Error = "内存空间不足";
                        return false;
                    }
                }
                else if (IsESC(Fm[Seek]))
                    Seek++;
                else
                {
                    Error = "存在非法字符：\"" + Fm[Seek] + "\"";
                    return false;
                }
            }
            Object data;
            if (!S_Number.PopStack(out data))
            {
                Error = "存在多余的运算符";
                return false;
            }
            out1 = (MultinomialC)data;
            Object O;
            while (S_Operator.PopStack(out O))
            {
                if (!S_Number.PopStack(out data))
                {
                    Error = "存在多余的运算符";
                    return false;
                }
                if (!Cell_Compute((Char)O, (MultinomialC)data, out1, out out1))
                    return false;
            }
            if (!S_Number.IsEmpty())
            {
                Error = "缺少运算符";
                return false;
            }
            for (int i = 0; i < out1.count; i++)
                if (Double.IsInfinity(out1.Buf[i].coefficient))
                {
                    Error = "数值太大，已溢出";
                    return false;
                }
            return true;
        }

        public override String ToString()           //把一个多项式转换为字符串
        {
            if (count == 0)
                return "0";
            String s = "";
            s += Buf[0].ToString();
            for (int i = 1; i < count; i++)
            {
                if (Buf[i].coefficient > 0)
                    s += "+";
                s += Buf[i].ToString();
            }
            return s;
        }

        static public bool Multinomial_Compute(String si, out String out1)      //带括号检查、括号替换和出错信息输出功能的多项式综合运算
        {
            out1 = "";
            if (si.Length == 0)
            {
                MessageBox.Show("请输入一个算式", "无法计算", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return false;
            }
            Formula f;
            try
            {
                f = new Formula(si, 0);
            }
            catch (Exception)
            {
                Error = "内存空间不足";
                return false;
            }
            if (!f.Check_Bracket())
            {
                MessageBox.Show("多项式非法：\n\n" + Error, "无法计算", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return false;
            }
            f.Formula_Adjust();
            Double d;
            if (f.Number_Compute(out d))
            {
                out1 = Formula.Number_Adjust(d);
                return true;
            }
            MultinomialC c;
            try
            {
                c = new MultinomialC(f.formula);
            }
            catch (Exception)
            {
                Error = "内存空间不足";
                return false;
            }
            Error = null;
            if (!c.Multinomial_Compute(out c))
            {
                if (Error == null)
                    MessageBox.Show("多项式非法", "无法计算", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                else
                    MessageBox.Show("多项式非法：\n\n" + Error, "无法计算", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return false;
            }
            out1 = c.ToString();
            return true;
        }
    }

    public class Stack
    {
        private int BM;         //基准存储容量
        private int top = -1;   //栈顶指针
        private int M;          //当前存储容量
        private Object[] Buf;   //栈空间

        public Stack(int m)     //构造函数
        {
            BM = M = m;
            try
            {
                Buf = new Object[M];
            }
            catch (Exception)
            {
                throw;
            }
        }

        private void Enlarge()  //扩充存储容量
        {
            Object[] old = Buf;
            try
            {
                Buf = new Object[M * 2];
            }
            catch (Exception)
            {
                throw;
            }
            for (int i = 0; i < M; i++)
                Buf[i] = old[i];
            M *= 2;
        }

        private void Reduce()   //缩减存储容量
        {
            if (M <= BM)
                return;
            Object[] old = Buf;
            M /= 2;
            try
            {
                Buf = new Object[M];
            }
            catch (Exception)
            {
                throw;
            }
            for (int i = 0; i < M; i++)
                Buf[i] = old[i];
        }

        public bool IsEmpty()   //判断栈是否为空
        {
            if (top < 0)
                return true;
            return false;
        }

        public void PushStack(Object a) //压栈元素a
        {
            if (top == M - 1)
                Enlarge();
            top++;
            Buf[top] = a;
        }

        public bool PopStack(out Object a)      //出栈一个元素给a，若栈空则返回FALSE
        {
            a = 0;
            if (top < 0)
                return false;
            a = Buf[top--];
            if (top < M / 4)
                Reduce();
            return true;
        }

        public bool GetTopData(out Object a)    //获取栈顶元素给a，若栈空则返回FALSE
        {
            a = 0;
            if (top < 0)
                return false;
            a = Buf[top];
            return true;
        }
    }

    public class Queue
    {
        int M;              //队列的存储容量
        String[] Buf;       //队列的存储空间
        int Head, Tail, Seek, State;
        public Queue(int n) //构造函数
        {
            M = n;
            try
            {
                Buf = new String[M];
            }
            catch (Exception)
            {
                throw;
            }
            Head = Tail = Seek = State = 0;
        }

        public bool InQueue1(String s)  //入队列，若队列满了则返回FALSE
        {
            if (State == 2)
                return false;
            Seek = Head;
            Buf[Head++] = s;
            if (Head >= M)
                Head = 0;
            if (Head == Tail)
                State = 2;
            else
                State = 1;
            return true;
        }

        public void InQueue2(String s)  //入队列，若队列满了则覆盖队尾元素
        {
            Seek = Head;
            Buf[Head++] = s;
            if (Head >= M)
                Head = 0;
            if (Head == Tail)
                Tail = (Head + 1) % M;
            State = 1;
        }

        public bool OutQueue(out String out1)       //出队列，若队列为空则返回FALSE
        {
            out1 = "";
            if (State == 0)
                return false;
            out1 = Buf[Tail++];
            if (Tail >= M)
                Tail = 0;
            if (Head == Tail)
                State = 0;
            if ((Seek + 1) % M == Tail)
                Seek = Tail;
            return true;
        }

        public bool Front_GetData(out String out1)  //获取游标的前一个元素，若队列为空则返回FALSE
        {
            out1 = "";
            if (State == 0)
                return false;
            Seek++;
            if (Seek >= M)
                Seek = 0;
            if (Seek == Head)
                Seek = Tail;
            out1 = Buf[Seek];
            return true;
        }

        public bool After_GetData(out String out1)  //获取游标的后一个元素，若队列为空则返回FALSE
        {
            out1 = "";
            if (State == 0)
                return false;
            if (Seek == Tail)
                Seek = Head - 1;
            else if (Seek == 0)
                Seek = M - 1;
            else
                Seek--;
            out1 = Buf[Seek];
            return true;
        }

        public bool GetHeadData(out String out1)    //获取队首元素，若队列为空则返回FALSE
        {
            out1 = "";
            if (State == 0)
                return false;
            int i = Head - 1;
            if (i == 0)
                i = M - 1;
            out1 = Buf[i];
            return true;
        }

        public bool GetTailData(out String out1)    //获取队尾元素，若队列为空则返回FALSE
        {
            out1 = "";
            if (State == 0)
                return false;
            out1 = Buf[Tail];
            return true;
        }

        public bool GetSeekData(out String out1)    //获取游标的当前元素，若队列为空则返回FALSE
        {
            out1 = "";
            if (State == 0)
                return false;
            out1 = Buf[Seek];
            return true;
        }
    }

    public class TableNode
    {
        public Double data; //数值
        public Double probability = -1; //概率
    }

    public class MyTable
    {
        public static String Error;//保存出错信息
        private int BM;     //存储空间的最小值
        private int M;      //存储空间的实际大小
        public int count = 0;   //记录的条数
        public TableNode[] Buf; //存储空间

        public MyTable(int m)//构造函数
        {
            BM = M = m;
            try
            {
                Buf = new TableNode[M];
            }
            catch (Exception)
            {
                throw;
            }
        }

        private void Enlarge()  //扩大存储空间
        {
            TableNode[] old = Buf;
            try
            {
                Buf = new TableNode[M * 2];
            }
            catch (Exception)
            {
                throw;
            }
            for (int i = 0; i < M; i++)
                Buf[i] = old[i];
            M *= 2;
        }

        private void Reduce()   //缩减存储空间
        {
            if (M <= BM)
                return;
            TableNode[] old = Buf;
            M /= 2;
            try
            {
                Buf = new TableNode[M];
            }
            catch (Exception)
            {
                throw;
            }
            for (int i = 0; i < M; i++)
                Buf[i] = old[i];
        }

        public void Add()//添加一个空元素
        {
            if (count == M)
                Enlarge();
            try
            {
                Buf[count++] = new TableNode();
            }
            catch (Exception)
            {
                throw;
            }
        }

        public void Add(Double d, Double p)///添加一个值为d和p元素
        {
            if (count == M)
                Enlarge();
            try
            {
                Buf[count++] = new TableNode();
            }
            catch (Exception)
            {
                throw;
            }
            Buf[count - 1].data = d;
            Buf[count - 1].probability = p;
        }

        public void Update1(int index, Double d)//更新索引为index的元素的数值
        {
            if (index < 0 || index >= count)
                return;
            Buf[index].data = d;
        }

        public void Update2(int index, Double p)//更新索引为index的元素的概率
        {
            if (index < 0 || index >= count)
                return;
            Buf[index].probability = p;
        }

        public void Remove(int index)//删除索引为index的元素
        {
            if (index < 0 || index >= count)
                return;
            for (int i = index + 1; i < count; i++)
                Buf[i - 1] = Buf[i];
            count--;
            if (count < M / 4)
                Reduce();
        }

        public void Clear()//删除所以元素
        {
            count = 0;
            M = BM;
            try
            {
                Buf = new TableNode[M];
            }
            catch (Exception)
            {
                throw;
            }
        }

        public bool Average(int n, out Double out1) //计算X^n的平均值
        {
            if (!SumN(n, out out1))
                return false;
            out1 /= count;
            return true;
        }

        public bool Expectation(out Double out1)    //计算数学期望
        {
            out1 = 0;
            Double d1 = 0;
            for (int i = 0; i < count; i++)
            {
                if (Buf[i].probability == -1)
                {
                    Error = "有些值没有输入概率";
                    return false;
                }
                d1 += Buf[i].probability;
            }
            if (d1 != 1)
            {
                Error = "总的概率：\"" + d1 + "\"≠\"1\"";
                return false;
            }
            for (int i = 0; i < count; i++)
                out1 += Buf[i].data * Buf[i].probability;
            return true;
        }

        public bool SumN(Double n, out Double out1)//计算X^n的和
        {
            out1 = 0;
            for (int i = 0; i < count; i++)
                out1 += Math.Pow(Buf[i].data, n);
            if (Double.IsInfinity(out1))
            {
                Error = "数值太大，已溢出";
                return false;
            }
            return true;
        }

        public bool Variance(out Double out1)//计算方差
        {
            if (!SumN(2, out out1))
                return false;
            Double d;
            if (!Average(1, out d))
                return false;
            out1 = (out1 - count * Math.Pow(d, 2)) / count;
            if (Double.IsInfinity(out1))
            {
                Error = "数值太大，已溢出";
                return false;
            }
            return true;
        }

        public bool StandardDeviation(out Double out1)//计算标准差
        {
            if (!Average(1, out out1))
                return false;
            Double d = 0;
            for (int i = 0; i < count; i++)
                d += Math.Pow(Buf[i].data - out1, 2);
            d /= count;
            out1 = Math.Pow(d, 0.5);
            if (Double.IsInfinity(out1))
            {
                Error = "数值太大，已溢出";
                return false;
            }
            return true;
        }
    }

    public class MyColorLoop : Label                //电阻的色环类
    {
        public int[] color;                         //该色环的选色条

        public MyColorLoop(int n)                   //构造函数
        {
            try
            {
                Font = new System.Drawing.Font("宋体", 28F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            }
            catch (Exception)
            {
                throw;
            }
            TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            Cursor = Cursors.Hand;
            try
            {
                switch (n)
                {
                    case 0:
                        color = new int[] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
                        break;
                    case 1:
                        color = new int[] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
                        break;
                    case 2:
                        color = new int[] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
                        break;
                    case 3:
                        color = new int[] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
                        break;
                    case 4:
                        color = new int[] { 1, 2, 5, 6, 7, 8, 10, 11, 12 };
                        break;
                }
            }
            catch (Exception)
            {
                throw;
            }
        }
    }

    public class MyColorBar : Label                 //选色条类
    {
        public MyColorBar(int n)                    //构造函数
        {
            TextAlign = ContentAlignment.MiddleCenter;
            Cursor = Cursors.Hand;
            Visible = false;
            switch (n)
            {
                case 0:
                    BackColor = Color.Black;
                    ForeColor = Color.White;
                    Text = "黑 色";
                    break;
                case 1:
                    BackColor = Color.Brown;
                    Text = "棕 色";
                    break;
                case 2:
                    BackColor = Color.Red;
                    Text = "红 色";
                    break;
                case 3:
                    BackColor = Color.Orange;
                    Text = "橙 色";
                    break;
                case 4:
                    BackColor = Color.Yellow;
                    Text = "黄 色";
                    break;
                case 5:
                    BackColor = Color.LimeGreen;
                    Text = "绿 色";
                    break;
                case 6:
                    BackColor = Color.Blue;
                    Text = "蓝 色";
                    break;
                case 7:
                    BackColor = Color.Purple;
                    Text = "紫 色";
                    break;
                case 8:
                    BackColor = Color.Gray;
                    Text = "灰 色";
                    break;
                case 9:
                    BackColor = Color.White;
                    Text = "白 色";
                    break;
                case 10:
                    BackColor = Color.Gold;
                    Text = "金 色";
                    break;
                case 11:
                    BackColor = Color.Silver;
                    Text = "银 色";
                    break;
                case 12:
                    BackColor = Color.Teal;
                    Text = "无 色";
                    break;
            }
        }
    }

    #endregion
}
