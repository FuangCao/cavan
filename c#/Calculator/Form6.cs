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
    public partial class Form6 : Form
    {
        private int Type=0;
        private int x, y;

        public Form6(int x,int y)
        {
            InitializeComponent();
            this.x = x;
            this.y = y;
        }

        private Double Unit_Double(String si)
        {
            if (si == "H")
                return 1;
            else if (si == "mH")
                return 1E-3;
            else if (si == "uH")
                return 1E-6;
            else if (si == "nH")
                return 1E-9;
            else if (si == "pH")
                return 1E-12;
            else if (si == "F")
                return 1;
            else if (si == "mF")
                return 1E-3;
            else if (si == "uF")
                return 1E-6;
            else if (si == "nF")
                return 1E-9;
            else if (si == "pF")
                return 1E-12;
            else if (si == "Hz")
                return 1;
            else if (si == "KHz")
                return 1E+3;
            else if (si == "MHz")
                return 1E+6;
            else if (si == "KMHz")
                return 1E+9;
            else
                return -1;
        }

        private void LC_F()
        {
            if (textBox_C.TextLength == 0 || comboBox_C.Text.Length == 0
                   || textBox_L.TextLength == 0 || comboBox_L.Text.Length == 0)
            {
                textBox_F.Text = "请输入电感和电容";
                return;
            }
            Double d1;
            if (!Formula.Number_Compute2(textBox_L.Text, out d1) || d1 <= 0)
            {
                textBox_F.Text = "Error";
                return;
            }
            d1 *= Unit_Double(comboBox_L.Text);
            Double d2;
            if (!Formula.Number_Compute2(textBox_C.Text, out d2) || d2 <= 0)
            {
                textBox_F.Text = "Error";
                return;
            }
            d2 *= Unit_Double(comboBox_C.Text);
            d1 = 1 / (2 * Math.PI * Math.Pow(d1 * d2, 0.5));
            if (d1 >= 1E+9)
            {
                d1 /= 1E+9;
                comboBox_F.Text = "KMHz";
            }
            else if (d1 >= 1E+6)
            {
                d1 /= 1E+6;
                comboBox_F.Text = "MHz";
            }
            else if (d1 >= 1E+3)
            {
                d1 /= 1E+3;
                comboBox_F.Text = "KHz";
            }
            else
                comboBox_F.Text = "Hz";
            textBox_F.Text = Math.Round(d1, 9).ToString();
        }

        private void LF_C()
        {
            if (textBox_L.TextLength == 0 || comboBox_L.Text.Length == 0
                   || textBox_F.TextLength == 0 || comboBox_F.Text.Length == 0)
            {
                textBox_C.Text = "请输入电感和频率";
                return;
            }
            Double d1;
            if (!Formula.Number_Compute2(textBox_L.Text,out d1) || d1 <= 0)
            {
                textBox_C.Text = "Error";
                return;
            }
            d1 *= Unit_Double(comboBox_L.Text);
            Double d2;
            if (!Formula.Number_Compute2(textBox_F.Text,out d2) || d2 <= 0)
            {
                textBox_C.Text = "Error";
                return;
            }
            d2 *= Unit_Double(comboBox_F.Text);
            d1 = 1 / (Math.Pow(2 * Math.PI * d2, 2) * d1);
            if (d1 <= 1E-9)
            {
                d1 *= 1E+12;
                comboBox_C.Text = "pF";
            }
            else if (d1 <= 1E-6)
            {
                d1 *= 1E+9;
                comboBox_C.Text = "nF";
            }
            else if (d1 <= 1E-3)
            {
                d1 *= 1E+6;
                comboBox_C.Text = "uF";
            }
            else if (d1 <= 1)
            {
                d1 *= 1E+3;
                comboBox_C.Text = "mF";
            }
            else
                comboBox_C.Text = "F";
            textBox_C.Text = Math.Round(d1, 9).ToString();
           
        }

        private void CF_L()
        {
            if (textBox_C.TextLength == 0 || comboBox_C.Text.Length == 0
                   || textBox_F.TextLength == 0 || comboBox_F.Text.Length == 0)
            {
                textBox_L.Text = "请输入电容和频率";
                return;
            }
            Double d1;
            if (!Formula.Number_Compute2(textBox_C.Text,out d1) || d1 <= 0)
            {
                textBox_L.Text = "Error";
                return;
            }
            d1 *= Unit_Double(comboBox_C.Text);
            Double d2;
            if (!Formula.Number_Compute2(textBox_F.Text,out d2) || d2 <= 0)
            {
                textBox_L.Text = "Error";
                return;
            }
            d2 *= Unit_Double(comboBox_F.Text);
            d1 = 1 / (Math.Pow(2 * Math.PI * d2, 2) * d1);
            if (d1 <= 1E-9)
            {
                d1 *= 1E+12;
                comboBox_L.Text = "pH";
            }
            else if (d1 <= 1E-6)
            {
                d1 *= 1E+9;
                comboBox_L.Text = "nH";
            }
            else if (d1 <= 1E-3)
            {
                d1 *= 1E+6;
                comboBox_L.Text = "uH";
            }
            else if (d1 <= 1)
            {
                d1 *= 1E+3;
                comboBox_L.Text = "mH";
            }
            else
                comboBox_L.Text = "H";
            textBox_L.Text = Math.Round(d1, 9).ToString();
            
        }

        private void textBox_L_TextChanged(object sender, EventArgs e)
        {
            if (Type == 1)
                return;
            if (Type == 0)
                LC_F();
            else if (Type == 2)
                LF_C();
        }

        private void textBox_C_TextChanged(object sender, EventArgs e)
        {
            if (Type == 2)
                return;
            if (Type == 0)
                LC_F();
            else if (Type == 1)
                CF_L();
            
        }

        private void textBox_F_TextChanged(object sender, EventArgs e)
        {
            if (Type == 0)
                return;
            if (Type == 1)
                CF_L();
            else if (Type == 2)
                LF_C();
        }

        private void radioButton_ComputeF_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton_ComputeF.Checked == true)
            {
                if (Type != 0)
                {
                    textBox_F.ForeColor = Color.Red;
                    comboBox_F.ForeColor = Color.Red;
                    label_F.ForeColor = Color.Red;
                    LC_F();
                }
                Type = 0;
            }
            else
            {
                Type = -1;
                textBox_F.ForeColor = Color.Black;
                comboBox_F.ForeColor = Color.Black;
                label_F.ForeColor = Color.Black;
                textBox_F.Clear();
            }
        }

        private void radioButton_ComputeL_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton_ComputeL.Checked == true)
            {
                if (Type != 1)
                {
                    textBox_L.ForeColor = Color.Red;
                    comboBox_L.ForeColor = Color.Red;
                    label_L.ForeColor = Color.Red;
                    CF_L();
                }
                Type = 1;
            }
            else
            {
                Type = -1;
                textBox_L.ForeColor = Color.Black;
                comboBox_L.ForeColor = Color.Black;
                label_L.ForeColor = Color.Black;
                textBox_L.Clear();
            }
        }

        private void radioButton_ComputeC_CheckedChanged(object sender, EventArgs e)
        {
            if (radioButton_ComputeC.Checked == true)
            {
                if (Type != 2)
                {
                    textBox_C.ForeColor = Color.Red;
                    comboBox_C.ForeColor = Color.Red;
                    label_C.ForeColor = Color.Red;
                    LF_C();
                }
                Type = 2;
            }
            else
            {
                Type = -1;
                textBox_C.ForeColor = Color.Black;
                comboBox_C.ForeColor = Color.Black;
                label_C.ForeColor = Color.Black;
                textBox_C.Clear();
            } 
        }

        private void Form6_Load(object sender, EventArgs e)
        {
            Top = y;
            Left = x;
            comboBox_C.Text = "pF";
            comboBox_L.Text = "pH";
            comboBox_F.Text = "Hz";
            LC_F();
        }
    }
}
