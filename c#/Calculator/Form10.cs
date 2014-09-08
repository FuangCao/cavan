using System;
using System.Windows.Forms;

namespace CFA090307A
{
    public partial class Form10 : Form
    {
        MyUnit Mu = new MyUnit();
        private int x, y;

        public Form10(int x,int y)
        {
            InitializeComponent();
            this.x = x;
            this.y = y;
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            comboBox_Type.SelectedIndex = 0;
            Top = y + 169;
            Left = x;
        }

        private void comboBox_Type_TextChanged(object sender, EventArgs e)
        {
            if (!Mu.CreatUnit(comboBox_Type.Text))
                return;
            comboBox_Unit1.Items.Clear();
            comboBox_Unit2.Items.Clear();
            for (int i = 0; i < Mu.count; i++)
            {
                comboBox_Unit1.Items.Add(Mu.units[i].unit);
                comboBox_Unit2.Items.Add(Mu.units[i].unit);
            }
            comboBox_Unit1.SelectedIndex = 0;
            comboBox_Unit2.SelectedIndex = 1;
        }

        public class Unit
        {
            public String unit;
            public Double value;
        }

        public class MyUnit
        {
            private const int M = 40;
            public Unit[] units = new Unit[M];
            public String Error;
            public int count = 0;

            public MyUnit()
            {
                for (int i = 0; i < M; i++)
                    units[i] = new Unit();
            }

            private void Add(Double value, String unit)
            {
                if (count >= M)
                    return;
                units[count].unit = unit;
                units[count].value = value;
                count++;
            }

            public bool CreatUnit(String s)
            {
                if (s == "长度")
                    ConstructLength();
                else if (s == "面积")
                    ConstructArea();
                else if (s == "重量")
                    ConstructWeight();
                else if (s == "体积")
                    ConstructCubage();
                else if (s == "力")
                    ConstructForce();
                else if (s == "功率")
                    ConstructPower();
                else if (s == "压强")
                    ConstructPressure();
                else if (s == "存储容量")
                    ConstructMemory();
                else if (s == "功能")
                    ConstructEnergy();
                else if (s == "时间")
                    ConstructTime();
                else
                    return false;
                return true;
            }

            private void ConstructLength()
            {
                count = 0;
                Add(1, "米");
                Add(10, "分米");
                Add(100, "厘米");
                Add(1000, "毫米");
                Add(1E+6, "微米");
                Add(1E+9, "纳米");
                Add(3, "市尺");
                Add(30, "市寸");
                Add(300, "市分");
                Add(3000, "市厘");
                Add(2E-3, "市里");
                Add(0.3, "丈");
                Add(0.03, "引");
                Add(1E-3, "千米");
                Add(1E-3, "公里");
                Add(0.621388E-3, "英里");
                Add(0.539665E-3, "海里");
                Add(39.37007874, "英寸");
                Add(3.280839895, "英尺");
                Add(1.0936, "码");
                Add(0.539956803E-3, "国际海里");
                Add(0.539607166E-3, "英海里");
                Add(0.539083558E-3, "美海里");
                Add(0.5468066, "英寻");
                Add(0.004971, "弗隆");
            }

            private void ConstructArea()
            {
                count = 0;
                Add(1, "平方米");
                Add(1, "公厘");
                Add(1E-4, "公顷");
                Add(1E-2, "公亩");
                Add(0.0015, "市亩");
                Add(0.00024711, "英亩");
                Add(1E-6, "平方千米");
                Add(1E-6, "平方公里");
                Add(1E+4, "平方厘米"); 
                Add(1E+2, "平方分米");
                Add(1E+6, "平方毫米");
                Add(0.386100386E-6, "平方英里");
                Add(9, "平方市尺");
                Add(10.7639104, "平方英尺");
                Add(1550.0031, "平方英寸");
                Add(0.0395369, "平方竿");
                Add(1.19599, "平方码");
            }

            private void ConstructWeight()
            {
                count = 0;
                Add(1, "千克"); 
                Add(1, "公斤");
                Add(1E-3, "吨"); 
                Add(1E+3, "克");
                Add(0.02, "市担");
                Add(0.01, "公担");
                Add(2, "市斤");
                Add(20, "市两");
                Add(200, "市钱"); 
                Add(2E+3, "市分");
                Add(2E+4, "市厘");
                Add(2E+5, "市毫");
                Add(2.204585538, "磅");
                Add(35.2739619, "盎司");
                Add(15432.3583529, "格令"); 
                Add(0.019684148, "英担"); 
                Add(0.984256613E-3, "(英制)长吨"); 
                Add(0.11023E-2, "(美制)短吨");
                Add(1E+6, "毫克");
                Add(2.6792289, "金衡磅");
                Add(643.0149314, "英钱");
                Add(32.1507466, "金衡盎司");
                Add(15432.3583529, "金衡格令");
                Add(0.0220462, "美担"); 
                Add(0.1574730, "英石"); 
                Add(564.3833912, "打兰"); 
            }

            private void ConstructTime()
            {
                count = 0;
                Add(1, "年");
                Add(12, "月");
                Add(365, "天");
                Add(8.76E+3, "小时");
                Add(5.256E+5, "分");
                Add(3.1536E+7, "秒");
                Add(3.1536E+10, "毫秒");
                Add(3.1536E+13, "微秒");
                Add(3.1536E+16, "纳秒");
            }

            private void ConstructForce()
            {
                count = 0;
                Add(1, "牛顿");
                Add(1E+5, "达因");
                Add(0.224770642201835, "磅");
                Add(0.101936799184506, "公斤");  
            }

            private void ConstructPower()
            {
                count = 0;
                Add(1,"千瓦");
                Add(1E+3, "瓦");
                Add(1.3596, "公制马力");
                Add(1.341, "英制马力");
                Add(102.0000007, "公斤·米/秒");
                Add(737.5627, "英尺·磅/秒");
                Add(0.239, "千卡/秒");
                Add(0.9478, "英热单位/秒");
                Add(1.36, "米制马力");
                Add(0.9478, "焦耳/秒");
                Add(1000, "牛顿·米/秒");
            }

            private void ConstructEnergy()
            {
                count = 0;
                Add(1, "千瓦·小时");
                Add(1, "度");
                Add(1.36, "马力·小时");
                Add(367170.6262985, "公斤·米");
                Add(2655147.5881968, "英尺·磅");
                Add(3599712.0230382, "焦耳");
                Add(1.3596112, "米制马力·时");
                Add(1.3408927, "英制马力·时");
                Add(859.9712023, "千卡");
                Add(3411.8070555, "英热单位");
            }

            private void ConstructPressure()
            {
                count = 0;
                Add(1, "公斤/平方厘米");
                Add(0.96784111, "标准大气压（帕）");
                Add(735.55924, "水银柱高度（毫米）");
                Add(10, "水柱高度（米）");
                Add(980.665, "毫巴");
                Add(14.223344, "磅力/平方英寸");
                Add(0.980665, "巴");
                Add(98.0665, "千帕");
                Add(980.665, "百帕");
                Add(98066.5, "帕斯卡");
                Add(735.55924, "毫米汞柱(托)");
                Add(2048.1615, "磅力/平方英尺");
                Add(28.959025, "英吋汞柱");
                Add(10000, "公斤/平方米");
                Add(10000, "毫米水柱");
            }

            private void ConstructMemory()
            {
                count = 0;
                Add(Math.Pow(2, 40), "字节（Byte）");
                Add(8 * Math.Pow(2, 40), "位（Bit）");
                Add(0.5 * Math.Pow(2, 40), "字（Word）");
                Add(0.25 * Math.Pow(2, 40), "双字（DWord）");
                Add(Math.Pow(2, 30), "KB");
                Add(Math.Pow(2, 20), "MB");
                Add(Math.Pow(2, 10), "GB");
                Add(1, "TB");
            }

            private void ConstructCubage()
            {
                count = 0;
                Add(1, "立方米");
                Add(20, "斛");
                Add(100, "斗");
                Add(1E+3, "合"); 
                Add(1E+5, "勺");
                Add(1E+6, "撮"); 
                Add(1E+3, "升"); 
                Add(1E+3, "立方分米");
                Add(27, "立方尺");
                Add(61023.7440947, "立方英寸");
                Add(35.3146667, "立方英尺");
                Add(1E+6, "立方厘米"); 
                Add(1E+6, "毫升"); 
                Add(219.974, "英加伦");
                Add(264.18, "美加仑");
                Add(879.9, "英夸脱");
                Add(1056.6882094, "美夸脱");
                Add(27.4967, "英蒲式耳");
                Add(10, "公石");
                Add(100, "十升");
                Add(10000, "分升");
                Add(100000, "厘升");
                Add(1000000000, "立方毫米");
                Add(8.6484898, "桶");
                Add(27.4961560, "蒲式耳");
                Add(219.9692483, "加仑");
                Add(1759.7539864, "品脱");
                Add(35195.0797279, "液量盎司");
                Add(200000.0/3.0, "汤勺");
                Add(200000, "调羹");
                Add(4226.7528377, "杯");
                Add(8453.5056755, "及耳");
                Add(270512.1816147, "液量打兰");
                Add(2077533554.801234, "量滴");
                Add(1.3079506, "立方码");
            }

            public bool Switch(Double di, String u1, String u2, out Double out1)
            {
                
                out1 = 0;
                if (di == 0)
                    return true;
                Double d1 = -1, d2 = -1;
                for (int i = 0; i < count; i++)
                {
                    if (units[i].unit == u1)
                    {
                        d1 = units[i].value;
                        if (d2 >= 0)
                            break;
                    }
                    if (units[i].unit == u2)
                    {
                        d2 = units[i].value;
                        if (d1 >= 0)
                            break;
                    }
                }
                if (d1 < 0)
                {
                    Error = "不存在单位\"" + u1 + "\"";
                    return false;
                }
                if (d2 < 0)
                {
                    Error = "不存在单位\"" + u2 + "\"";
                    return false;
                }
                out1 = d2 * di / d1;
                return true;
            }
        }

        private void textBox_Value1_TextChanged(object sender, EventArgs e)
        {
            if (textBox_Value1.TextLength == 0)
            {
                label_Value2.Text = "请输入源数据大小";
                return;
            }
            if(comboBox_Unit1.Text.Length == 0)
            {
                label_Value2.Text = "请输入源单位";
                return;
            }
            if (comboBox_Unit2.Text.Length == 0)
            {
                label_Value2.Text = "请输入目的单位";
                return;
            }
            Double d1;
            if(!Formula.Number_Compute2(textBox_Value1.Text,out d1))
            {
                label_Value2.Text = Formula.ErrorText;
                return;
            }
            Double d2;
            if(!Mu.Switch(d1,comboBox_Unit1.Text,comboBox_Unit2.Text,out d2))
            {
                label_Value2.Text = "Error";
                return;
            }
            label_Value2.Text = "= " + Formula.Number_Adjust(d2) + "(" + comboBox_Unit2.Text + ")";
        }
    }
}
