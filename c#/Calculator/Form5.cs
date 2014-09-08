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
    public partial class Form5 : Form
    {
        int x, y;
        Double[] Resistance = new Double[50];
        Double[] data = new Double[50];
        public Form5(int x,int y)
        {
            InitializeComponent();
            this.x = x;
            this.y = y;
        }

        private void dataGridView1_DefaultValuesNeeded(object sender, DataGridViewRowEventArgs e)
        {
            e.Row.Cells[0].Value = e.Row.Index;
            if (e.Row.Cells[2].Value != null)
                return;
            DataGridViewComboBoxCell Cell = e.Row.Cells[2] as DataGridViewComboBoxCell;
            Cell.Items.AddRange("Ω", "KΩ", "MΩ", "KMΩ");
            e.Row.Cells[2].Value = Cell.Items[0];
            
        }

        public int Compute(out Double out1)
        {
            int length = dataGridView1.Rows.Count - 1;
            Double d = 0;
            out1 = 1;
            for (int i = 0; i < length; i++)
            {
                out1 *= Resistance[i];
                Double d1 = 1;
                for (int j = 0; j < i; j++)
                    d1 *= Resistance[j];
                for (int j = i + 1; j < length; j++)
                    d1 *= Resistance[j];
                d += d1;
            }
            out1 /= d;
            int n = 0;
            if (out1 > 1E+9)
            {
                out1 /= 1E+9;
                n = 3;
            }
            else if (out1 > 1E+6)
            {
                out1 /= 1E+6;
                n = 2;
            }
            else if (out1 > 1E+3)
            {
                out1 /= 1E+3;
                n = 1;
            }
            data[length] = Resistance[length] = out1;
            return n;
        }

        private void button_Compute_Click(object sender, EventArgs e)
        {
            DataGridViewComboBoxCell Cell = dataGridView1[2, dataGridView1.NewRowIndex] as DataGridViewComboBoxCell;
            if (Cell.Items.Count == 0)
                Cell.Items.AddRange("Ω", "KΩ", "MΩ", "KMΩ");
            dataGridView1[0, dataGridView1.NewRowIndex].Value = "计算结果";
            dataGridView1[2, dataGridView1.NewRowIndex].Value = Cell.Items[Compute(out Resistance[dataGridView1.Rows.Count - 1])];
            dataGridView1[1, dataGridView1.NewRowIndex].Value = Math.Round(Resistance[dataGridView1.Rows.Count - 1], 5);
        }

        private void button_Clear_Click(object sender, EventArgs e)
        {
            dataGridView1.Rows.Clear();
        }

        private void button_Remove_Click(object sender, EventArgs e)
        {
            int i = dataGridView1.SelectedRows[0].Index;
            if (dataGridView1.SelectedRows.Count == 0)
                return;
            try
            {
                dataGridView1.Rows.Remove(dataGridView1.SelectedRows[0]);
            }
            catch (Exception) { return; }
            int length = dataGridView1.RowCount - 1;
            for (; i < length; i++)
            {
                Resistance[i] = Resistance[i + 1];
                data[i] = data[i + 1];
                dataGridView1[0, i].Value = i;
            }
            if (dataGridView1.RowCount > 1)
                button_Compute_Click(0, new EventArgs());
        }

        private void Form5_Load(object sender, EventArgs e)
        {
            Top = y;
            Left = x;
        }

        private void dataGridView1_CellValidating(object sender, DataGridViewCellValidatingEventArgs e)
        {
            if (e.ColumnIndex == 0)
                return;
            if (e.ColumnIndex == 1)
            {
                String s = e.FormattedValue.ToString();
                if (s.Length == 0 || s == "null")
                {
                    dataGridView1.Rows[e.RowIndex].ErrorText = "阻值不能为空";
                    MessageBox.Show("阻值不能为空");
                    e.Cancel = true;
                    return;
                }
                if (s != dataGridView1[1, e.RowIndex].FormattedValue.ToString())
                {
                    
                    if (!Formula.Number_Compute(s,0, out data[e.RowIndex]))
                    {
                        dataGridView1.Rows[e.RowIndex].ErrorText = Formula.ErrorText;
                        e.Cancel = true;
                        return;
                    }
                    if (data[e.RowIndex] < 0)
                    {
                        dataGridView1.Rows[e.RowIndex].ErrorText = "阻值不能为负数";
                        MessageBox.Show("阻值不能为负数");
                        e.Cancel = true;
                        return;
                    }
                    Resistance[e.RowIndex] = data[e.RowIndex] * Form1.Unit_Int(dataGridView1[2, e.RowIndex].FormattedValue.ToString());
                }
            }
            else if (e.ColumnIndex == 2)
            {
                String s = e.FormattedValue.ToString();
                if (s != dataGridView1[2, e.RowIndex].FormattedValue.ToString())
                    Resistance[e.RowIndex] = data[e.RowIndex] * Form1.Unit_Int(s);
            }
        }

        private void dataGridView1_CellValidated(object sender, DataGridViewCellEventArgs e)
        {
            dataGridView1.Rows[e.RowIndex].ErrorText = String.Empty;
            if (e.ColumnIndex > 0)
                button_Compute_Click(0, new EventArgs());
        }

        private void dataGridView1_RowValidating(object sender, DataGridViewCellCancelEventArgs e)
        {
            if (dataGridView1[1, e.RowIndex].Value == null || dataGridView1[1, e.RowIndex].Value.ToString().Length == 0)
            {
                dataGridView1.Rows[e.RowIndex].ErrorText = "阻值不能为空";
                MessageBox.Show("阻值不能为空");
                e.Cancel = true;
                return;
            }
        }
    }
}
