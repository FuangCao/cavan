namespace CFA090307A
{
    partial class Form10
    {
        /// <summary>
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows 窗体设计器生成的代码

        /// <summary>
        /// 设计器支持所需的方法 - 不要
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form10));
            this.comboBox_Type = new System.Windows.Forms.ComboBox();
            this.comboBox_Unit1 = new System.Windows.Forms.ComboBox();
            this.comboBox_Unit2 = new System.Windows.Forms.ComboBox();
            this.textBox_Value1 = new System.Windows.Forms.TextBox();
            this.label_Value2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // comboBox_Type
            // 
            this.comboBox_Type.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox_Type.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.comboBox_Type.FormattingEnabled = true;
            this.comboBox_Type.Items.AddRange(new object[] {
            "长度",
            "存储容量",
            "功率",
            "功能",
            "力",
            "面积",
            "时间",
            "体积",
            "压强",
            "重量"});
            this.comboBox_Type.Location = new System.Drawing.Point(61, 36);
            this.comboBox_Type.MaxDropDownItems = 18;
            this.comboBox_Type.Name = "comboBox_Type";
            this.comboBox_Type.Size = new System.Drawing.Size(139, 22);
            this.comboBox_Type.Sorted = true;
            this.comboBox_Type.TabIndex = 0;
            this.comboBox_Type.TextChanged += new System.EventHandler(this.comboBox_Type_TextChanged);
            // 
            // comboBox_Unit1
            // 
            this.comboBox_Unit1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox_Unit1.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.comboBox_Unit1.FormattingEnabled = true;
            this.comboBox_Unit1.Location = new System.Drawing.Point(300, 79);
            this.comboBox_Unit1.MaxDropDownItems = 28;
            this.comboBox_Unit1.Name = "comboBox_Unit1";
            this.comboBox_Unit1.Size = new System.Drawing.Size(134, 22);
            this.comboBox_Unit1.Sorted = true;
            this.comboBox_Unit1.TabIndex = 1;
            this.comboBox_Unit1.TextChanged += new System.EventHandler(this.textBox_Value1_TextChanged);
            // 
            // comboBox_Unit2
            // 
            this.comboBox_Unit2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox_Unit2.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.comboBox_Unit2.FormattingEnabled = true;
            this.comboBox_Unit2.Location = new System.Drawing.Point(300, 36);
            this.comboBox_Unit2.MaxDropDownItems = 28;
            this.comboBox_Unit2.Name = "comboBox_Unit2";
            this.comboBox_Unit2.Size = new System.Drawing.Size(134, 22);
            this.comboBox_Unit2.Sorted = true;
            this.comboBox_Unit2.TabIndex = 2;
            this.comboBox_Unit2.TextChanged += new System.EventHandler(this.textBox_Value1_TextChanged);
            // 
            // textBox_Value1
            // 
            this.textBox_Value1.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.textBox_Value1.Location = new System.Drawing.Point(61, 79);
            this.textBox_Value1.Name = "textBox_Value1";
            this.textBox_Value1.Size = new System.Drawing.Size(139, 23);
            this.textBox_Value1.TabIndex = 3;
            this.textBox_Value1.TextChanged += new System.EventHandler(this.textBox_Value1_TextChanged);
            // 
            // label_Value2
            // 
            this.label_Value2.AutoSize = true;
            this.label_Value2.Font = new System.Drawing.Font("宋体", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label_Value2.ForeColor = System.Drawing.Color.Red;
            this.label_Value2.Location = new System.Drawing.Point(5, 9);
            this.label_Value2.Name = "label_Value2";
            this.label_Value2.Size = new System.Drawing.Size(56, 16);
            this.label_Value2.TabIndex = 4;
            this.label_Value2.Text = "label1";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label1.Location = new System.Drawing.Point(5, 39);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(49, 14);
            this.label1.TabIndex = 5;
            this.label1.Text = "类型：";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label2.Location = new System.Drawing.Point(217, 39);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(77, 14);
            this.label2.TabIndex = 6;
            this.label2.Text = "目的单位：";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label3.Location = new System.Drawing.Point(217, 82);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(77, 14);
            this.label3.TabIndex = 7;
            this.label3.Text = "源 单 位：";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label4.Location = new System.Drawing.Point(6, 82);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(49, 14);
            this.label4.TabIndex = 8;
            this.label4.Text = "大小：";
            // 
            // Form10
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.MistyRose;
            this.ClientSize = new System.Drawing.Size(456, 117);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.label_Value2);
            this.Controls.Add(this.textBox_Value1);
            this.Controls.Add(this.comboBox_Unit2);
            this.Controls.Add(this.comboBox_Unit1);
            this.Controls.Add(this.comboBox_Type);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "Form10";
            this.Text = "单位换算 计Y062班 曹福昂";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox comboBox_Type;
        private System.Windows.Forms.ComboBox comboBox_Unit1;
        private System.Windows.Forms.ComboBox comboBox_Unit2;
        private System.Windows.Forms.Label label_Value2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        public System.Windows.Forms.TextBox textBox_Value1;
    }
}

