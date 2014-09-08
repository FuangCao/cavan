using System;
using System.Windows.Forms;

namespace CFA090307A
{
    public partial class Form2 : Form
    {
        int x, y;
        public Form2(int x, int y)
        {
            InitializeComponent();
            this.x = x;
            this.y = y;
        }

        public String DeleteZero(String si)
        {
            if (si.Length == 0)
                return "";
            int n = si.IndexOf('.');
            if (n == -1)
                n = si.Length;
            int a = 0;
            while (a < n && si[a] == '0')
                a++;
            String s1 = "";
            for (int i = a; i < n; i++)
                s1 += si[i];
            a = si.Length - 1;
            while (a > n && si[a] == '0')
                a--;
            String s2 = "";
            for (int i = n + 1; i <= a; i++)
                s2 += si[i];
            if (s1.Length == 0)
                return "0." + s2;
            else if (s2.Length == 0)
                return s1;
            else
                return s1 + "." + s2;
        }

        public String Head_Tail(String si)
        {
            String s = "";
            for (int i = si.Length - 1; i >= 0; i--)
                s += si[i];
            return s;
        }

        public bool B_D_High(String s, out ulong out1)
        {
            out1 = 0;
            for (int i = 0; i < s.Length; i++)
            {
                out1 <<= 1;
                if (s[i] == '1')
                    out1 += 1;
                else if (s[i] != '0')
                    return false;
            }
            return true;
        }

        public bool B_D_Low(String s, out decimal out1)
        {
            out1 = 0;
            decimal d = 0.5M;
            for (int i = 0; i < s.Length; i++)
            {
                if (s[i] == '1')
                    out1 += d;
                else if (s[i] != '0')
                    return false;
                d *= 0.5M;
            }
            return true;
        }

        public bool B_D(String si, out String out1)
        {
            out1 = "";
            if (si.Length == 0)
                return true;
            if (si[0] == '.')
                return false;
            int n = si.IndexOf('.');
            if (n == si.Length - 1)
                return false;
            if (n != si.LastIndexOf('.'))
                return false;
            if (n == -1)
                n = si.Length;
            String s1 = "";
            for (int i = 0; i < n; i++)
                s1 += si[i];
            ulong a;
            if (!B_D_High(s1, out a))
                return false;
            out1 = a.ToString();
            if (n == si.Length)
                return true;
            s1 = "";
            for (int i = n + 1; i < si.Length; i++)
                s1 += si[i];
            decimal d;
            if (!B_D_Low(s1, out d))
                return false;
            if (d == 0)
                return true;
            s1 = Math.Round(d, 25).ToString();
            out1 = DeleteZero(out1 + s1.Remove(0, 1));
            return true;
        }

        public bool B_O(String si, out String out1)
        {
            out1 = "";
            if (si.Length == 0)
                return true;
            if (si[0] == '.')
                return false;
            int n = si.IndexOf('.');
            if (n == si.Length - 1)
                return false;
            if (n != si.LastIndexOf('.'))
                return false;
            if (n == -1)
                n = si.Length;
            String s1 = "";
            for (int i = n - 1; i >= 0; i -= 3)
            {
                s1 = "";
                for (int j = i - 2; j <= i; j++)
                {
                    if (j >= 0)
                        s1 += si[j];
                    else
                        s1 += '0';
                }
                ulong a;
                if (B_D_High(s1, out a))
                    out1 += a.ToString();
                else
                    return false;

            }
            s1 = out1;
            out1 = "";
            int k = s1.Length - 1;
            for (; k >= 0; k--)
            {
                if (s1[k] != '0')
                    break;
            }
            for (; k >= 0; k--)
                out1 += s1[k];
            if (out1 == "")
                out1 = "0";
            if (n == si.Length)
                return true;
            String s2 = "";
            for (int i = n + 1; i < si.Length; i += 3)
            {
                s1 = "";
                for (int j = i; j < i + 3; j++)
                {
                    if (j < si.Length)
                        s1 += si[j];
                    else
                        s1 += '0';
                }
                ulong a;
                if (B_D_High(s1, out a))
                    s2 += a.ToString();
                else
                    return false;

            }
            k = s2.Length - 1;
            for (; k >= 0; k--)
            {
                if (s2[k] != '0')
                    break;
            }
            if (k < 0)
                return true;
            out1 += '.';
            for (int i = 0; i <= k; i++)
                out1 += s2[i];
            return true;
        }

        public bool B_H(String si, out String out1)
        {
            out1 = "";
            if (si.Length == 0)
                return true;
            if (si[0] == '.')
                return false;
            int n = si.IndexOf('.');
            if (n == si.Length - 1)
                return false;
            if (n != si.LastIndexOf('.'))
                return false;
            if (n == -1)
                n = si.Length;
            String s1 = "";
            for (int i = n - 1; i >= 0; i -= 4)
            {
                s1 = "";
                for (int j = i - 3; j <= i; j++)
                {
                    if (j >= 0)
                        s1 += si[j];
                    else
                        s1 += '0';
                }
                ulong a;
                if (B_D_High(s1, out a))
                {
                    if (a > 9)
                        out1 += (Char)(a - 10 + 'A');
                    else
                        out1 += a.ToString();
                }
                else
                    return false;
            }
            s1 = out1;
            out1 = "";
            int k = s1.Length - 1;
            for (; k >= 0; k--)
            {
                if (s1[k] != '0')
                    break;
            }
            for (; k >= 0; k--)
                out1 += s1[k];
            if (out1 == "")
                out1 = "0";
            if (n == si.Length)
                return true;
            String s2 = "";
            for (int i = n + 1; i < si.Length; i += 4)
            {
                s1 = "";
                for (int j = i; j < i + 4; j++)
                {
                    if (j < si.Length)
                        s1 += si[j];
                    else
                        s1 += '0';
                }
                ulong a;
                if (B_D_High(s1, out a))
                {
                    if (a > 9)
                        s2 += (Char)(a - 10 + 'A');
                    else
                        s2 += a.ToString();
                }
                else
                    return false;
            }
            k = s2.Length - 1;
            for (; k >= 0; k--)
            {
                if (s2[k] != '0')
                    break;
            }
            if (k < 0)
                return true;
            out1 += '.';
            for (int i = 0; i <= k; i++)
                out1 += s2[i];
            return true;
        }

        public bool O_B(int a, out String out1)
        {
            if (a == 0)
                out1 = "000";
            else if (a == 1)
                out1 = "001";
            else if (a == 2)
                out1 = "010";
            else if (a == 3)
                out1 = "011";
            else if (a == 4)
                out1 = "100";
            else if (a == 5)
                out1 = "101";
            else if (a == 6)
                out1 = "110";
            else if (a == 7)
                out1 = "111";
            else
            {
                out1 = "";
                return false;
            }
            return true;
        }

        public bool O_B(String si, out String out1)
        {
            out1 = "";
            if (si.Length == 0)
                return true;
            if (si[0] == '.' || si[si.Length - 1] == '.')
                return false;
            if (si.IndexOf('.') != si.LastIndexOf('.'))
                return false;
            String s1;
            for (int i = 0; i < si.Length; i++)
            {
                if (si[i] == '.')
                    out1 += '.';
                else
                {
                    if (!O_B(si[i] - '0', out s1))
                        return false;
                    out1 += s1;
                }
            }
            out1 = DeleteZero(out1);
            return true;
        }

        public bool O_D(String si, out String out1)
        {
            out1 = "";
            String s;
            if (!O_B(si, out s))
                return false;
            if (!B_D(s, out out1))
                return false;
            return true;
        }

        public bool O_H(String si, out String out1)
        {
            out1 = "";
            String s;
            if (!O_B(si, out s))
                return false;
            if (!B_H(s, out out1))
                return false;
            return true;
        }

        public bool D_B(String si, out String out1)
        {
            out1 = "";
            if (si.Length == 0)
                return true;
            if (si[0] == '.')
                return false;
            int n = si.IndexOf('.');
            if (n == si.Length - 1)
                return false;
            if (n != si.LastIndexOf('.'))
                return false;
            if (n == -1)
                n = si.Length;
            ulong a = 0;
            for (int i = 0; i < n; i++)
            {
                a *= 10;
                a += (ulong)(si[i] - '0');
            }
            while (a > 0)
            {
                if ((a & 1) == 1)
                    out1 += '1';
                else
                    out1 += '0';
                a >>= 1;
            }
            if (out1.Length == 0)
                out1 = "0";
            else
                out1 = Head_Tail(out1);
            if (n == si.Length)
                return true;
            decimal d1, d2;
            d1 = 0;
            d2 = 0.1M;
            for (int i = n + 1; i < si.Length; i++)
            {
                d1 += (si[i] - '0') * d2;
                d2 *= 0.1M;
            }
            out1 += '.';
            for (int j = 0; j < 500; j++)
            {
                if (d1 == 0)
                    break;
                d1 *= 2;
                if (d1 < 1)
                    out1 += "0";
                else
                {
                    out1 += "1";
                    d1 -= 1;
                }
            }
            return true;
        }

        public bool D_O(String si, out String out1)
        {
            out1 = "";
            String s;
            if (!D_B(si, out s))
                return false;
            if (!B_O(s, out out1))
                return false;
            return true;
        }

        public bool D_H(String si, out String out1)
        {
            out1 = "";
            String s;
            if (!D_B(si, out s))
                return false;
            if (!B_H(s, out out1))
                return false;
            return true;
        }

        public bool H_B(int a, out String out1)
        {
            if (a >= 0 && a <= 7)
            {
                O_B(a, out out1);
                out1 = "0" + out1;
            }
            else if (a >= 8 && a <= 15)
            {
                O_B(a - 8, out out1);
                out1 = "1" + out1;
            }
            else
            {
                out1 = "";
                return false;
            }
            return true;
        }

        public bool H_B(String si, out String out1)
        {
            out1 = "";
            if (si.Length == 0)
                return true;
            if (si[0] == '.' || si[si.Length - 1] == '.')
                return false;
            if (si.IndexOf('.') != si.LastIndexOf('.'))
                return false;
            String s1;
            for (int i = 0; i < si.Length; i++)
            {
                if (si[i] == '.')
                    out1 += '.';
                else
                {
                    int a = 0;
                    if (si[i] >= '0' && si[i] <= '9')
                        a = si[i] - '0';
                    else if (si[i] >= 'A' && si[i] <= 'F')
                        a = 10 + si[i] - 'A';
                    else if (si[i] >= 'a' && si[i] <= 'f')
                        a = 10 + si[i] - 'a';
                    else
                        return false;
                    if (!H_B(a, out s1))
                        return false;
                    out1 += s1;
                }
            }
            out1 = DeleteZero(out1);
            return true;
        }

        public bool H_O(String si, out String out1)
        {
            out1 = "";
            String s;
            if (!H_B(si, out s))
                return false;
            if (!B_O(s, out out1))
                return false;
            return true;
        }

        public bool H_D(String si, out String out1)
        {
            out1 = "";
            String s;
            if (!H_B(si, out s))
                return false;
            if (!B_D(s, out out1))
                return false;
            return true;
        }

        private void textBox_binary_TextChanged(object sender, EventArgs e)
        {
            if (!textBox_binary.Focused)
                return;
            textBox_octal.Text = textBox_decimal.Text = textBox_hex.Text = "Error";
            if (textBox_binary.TextLength == 0)
                return;
            String s = textBox_binary.Text;
            for (int i = 0; i < s.Length; i++)
            {
                if (!(s[i] == '1' || s[i] == '0' || s[i] == '.'))
                {
                    MessageBox.Show("您的输入有误");
                    textBox_binary.Text = "";
                    return;
                }
            }
            if (B_D(textBox_binary.Text, out s))
                textBox_decimal.Text = s;
            if (B_O(textBox_binary.Text, out s))
                textBox_octal.Text = s;
            if (B_H(textBox_binary.Text, out s))
                textBox_hex.Text = s;
        }

        private void textBox_octal_TextChanged(object sender, EventArgs e)
        {
            if (!textBox_octal.Focused)
                return;
            textBox_binary.Text = textBox_decimal.Text = textBox_hex.Text = "Error";
            if (textBox_octal.TextLength == 0)
                return;
            String s = textBox_octal.Text;
            for (int i = 0; i < s.Length; i++)
            {
                if (!(s[i] >= '0' && s[i] <= '7' || s[i] == '.'))
                {
                    MessageBox.Show("您的输入有误");
                    textBox_octal.Text = "";
                    return;
                }
            }
            if (O_B(textBox_octal.Text, out s))
                textBox_binary.Text = s;
            else
                return;
            String s1;
            if (B_D(s, out s1))
                textBox_decimal.Text = s1;
            if (B_H(s, out s1))
                textBox_hex.Text = s1;
        }

        public void textBox_decimal_TextChanged(object sender, EventArgs e)
        {
            if (!textBox_decimal.Focused)
                return;
            textBox_binary.Text = textBox_octal.Text = textBox_hex.Text = "Error";
            if (textBox_decimal.TextLength == 0)
                return;
            Double d;
            if (!Formula.Number_Compute2(textBox_decimal.Text, out d) || d < 0)
                return;
            String s;
            if (D_B(d.ToString(), out s))
                textBox_binary.Text = s;
            else
                return;
            String s1;
            if (B_O(s, out s1))
                textBox_octal.Text = s1;
            if (B_H(s, out s1))
                textBox_hex.Text = s1;
        }

        private void textBox_hex_TextChanged(object sender, EventArgs e)
        {
            if (!textBox_hex.Focused)
                return;
            textBox_binary.Text = textBox_octal.Text = textBox_decimal.Text = "Error";
            if (textBox_hex.TextLength == 0)
                return;
            String s = textBox_hex.Text;
            for (int i = 0; i < s.Length; i++)
            {
                if (!(s[i] >= '0' && s[i] <= '9' || s[i] >= 'A' && s[i] <= 'F' || s[i] >= 'a' && s[i] <= 'f' || s[i] == '.'))
                {
                    MessageBox.Show("您的输入有误");
                    textBox_hex.Text = "";
                    return;
                }
            }
            if (H_B(textBox_hex.Text, out s))
                textBox_binary.Text = s;
            else
                return;
            String s1;
            if (B_O(s, out s1))
                textBox_octal.Text = s1;
            if (B_D(s, out s1))
                textBox_decimal.Text = s1;
        }

        private void button_SHL_Click(object sender, EventArgs e)
        {
            textBox_binary.Focus();
            if (textBox_binary.TextLength == 0)
                return;
            String s = textBox_binary.Text.Remove(0, 1);
            if (s.Length > 0 && s[0] == '.')
                s = s.Remove(0, 1);
            textBox_binary.Text = s + '0';
            textBox_binary.SelectionStart = 0;
        }

        private void button_SHR_Click(object sender, EventArgs e)
        {
            textBox_binary.Focus();
            if (textBox_binary.TextLength == 0)
                return;
            String s = textBox_binary.Text.Remove(textBox_binary.TextLength - 1, 1);
            if (s.Length > 0 && s[s.Length - 1] == '.')
                s = s.Remove(s.Length - 1, 1);
            textBox_binary.Text = '0' + s;
            textBox_binary.SelectionStart = textBox_binary.TextLength;
        }

        private void button_SAR_Click(object sender, EventArgs e)
        {
            textBox_binary.Focus();
            if (textBox_binary.TextLength == 0)
                return;
            String s = textBox_binary.Text.Remove(textBox_binary.TextLength - 1, 1);
            if (s.Length > 0 && s[s.Length - 1] == '.')
                s = s.Remove(s.Length - 1, 1);
            textBox_binary.Text = textBox_binary.Text[0] + s;
            textBox_binary.SelectionStart = textBox_binary.TextLength;
        }

        private void button_ROL_Click(object sender, EventArgs e)
        {
            textBox_binary.Focus();
            if (textBox_binary.TextLength == 0)
                return;
            String s = textBox_binary.Text.Remove(0, 1);
            if (s.Length > 0 && s[0] == '.')
                s = s.Remove(0, 1);
            textBox_binary.Text = s + textBox_binary.Text[0];
            textBox_binary.SelectionStart = 0;
        }

        private void button_ROR_Click(object sender, EventArgs e)
        {
            textBox_binary.Focus();
            if (textBox_binary.TextLength == 0)
                return;
            String s = textBox_binary.Text.Remove(textBox_binary.TextLength - 1, 1);
            if (s.Length > 0 && s[s.Length - 1] == '.')
                s = s.Remove(s.Length - 1, 1);
            textBox_binary.Text = textBox_binary.Text[textBox_binary.TextLength - 1] + s;
            textBox_binary.SelectionStart = textBox_binary.TextLength;
        }

        private void Form2_Load(object sender, EventArgs e)
        {
            Top = y;
            Left = x;
        }

    }
}
