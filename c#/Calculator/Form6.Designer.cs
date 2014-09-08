namespace CFA090307A
{
    partial class Form6
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form6));
            this.label_L = new System.Windows.Forms.Label();
            this.label_C = new System.Windows.Forms.Label();
            this.label_F = new System.Windows.Forms.Label();
            this.textBox_L = new System.Windows.Forms.TextBox();
            this.textBox_C = new System.Windows.Forms.TextBox();
            this.textBox_F = new System.Windows.Forms.TextBox();
            this.comboBox_L = new System.Windows.Forms.ComboBox();
            this.comboBox_C = new System.Windows.Forms.ComboBox();
            this.comboBox_F = new System.Windows.Forms.ComboBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.radioButton_ComputeC = new System.Windows.Forms.RadioButton();
            this.radioButton_ComputeL = new System.Windows.Forms.RadioButton();
            this.radioButton_ComputeF = new System.Windows.Forms.RadioButton();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // label_L
            // 
            this.label_L.AutoSize = true;
            this.label_L.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label_L.Location = new System.Drawing.Point(18, 60);
            this.label_L.Name = "label_L";
            this.label_L.Size = new System.Drawing.Size(56, 14);
            this.label_L.TabIndex = 0;
            this.label_L.Text = "电感 L=";
            // 
            // label_C
            // 
            this.label_C.AutoSize = true;
            this.label_C.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label_C.Location = new System.Drawing.Point(18, 101);
            this.label_C.Name = "label_C";
            this.label_C.Size = new System.Drawing.Size(56, 14);
            this.label_C.TabIndex = 1;
            this.label_C.Text = "电容 C=";
            // 
            // label_F
            // 
            this.label_F.AutoSize = true;
            this.label_F.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.label_F.ForeColor = System.Drawing.Color.Red;
            this.label_F.Location = new System.Drawing.Point(18, 142);
            this.label_F.Name = "label_F";
            this.label_F.Size = new System.Drawing.Size(56, 14);
            this.label_F.TabIndex = 2;
            this.label_F.Text = "频率 F=";
            // 
            // textBox_L
            // 
            this.textBox_L.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.textBox_L.Location = new System.Drawing.Point(80, 56);
            this.textBox_L.Name = "textBox_L";
            this.textBox_L.Size = new System.Drawing.Size(137, 23);
            this.textBox_L.TabIndex = 3;
            this.toolTip1.SetToolTip(this.textBox_L, "可以是一个算式，但值不能为负数");
            this.textBox_L.TextChanged += new System.EventHandler(this.textBox_L_TextChanged);
            // 
            // textBox_C
            // 
            this.textBox_C.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.textBox_C.Location = new System.Drawing.Point(80, 97);
            this.textBox_C.Name = "textBox_C";
            this.textBox_C.Size = new System.Drawing.Size(137, 23);
            this.textBox_C.TabIndex = 4;
            this.toolTip1.SetToolTip(this.textBox_C, "可以是一个算式，但值不能为负数");
            this.textBox_C.TextChanged += new System.EventHandler(this.textBox_C_TextChanged);
            // 
            // textBox_F
            // 
            this.textBox_F.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.textBox_F.ForeColor = System.Drawing.Color.Red;
            this.textBox_F.Location = new System.Drawing.Point(80, 138);
            this.textBox_F.Name = "textBox_F";
            this.textBox_F.Size = new System.Drawing.Size(137, 23);
            this.textBox_F.TabIndex = 5;
            this.toolTip1.SetToolTip(this.textBox_F, "可以是一个算式，但值不能为负数");
            this.textBox_F.TextChanged += new System.EventHandler(this.textBox_F_TextChanged);
            // 
            // comboBox_L
            // 
            this.comboBox_L.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox_L.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.comboBox_L.FormattingEnabled = true;
            this.comboBox_L.Items.AddRange(new object[] {
            "H",
            "mH",
            "uH",
            "nH",
            "pH"});
            this.comboBox_L.Location = new System.Drawing.Point(223, 57);
            this.comboBox_L.Name = "comboBox_L";
            this.comboBox_L.Size = new System.Drawing.Size(63, 22);
            this.comboBox_L.TabIndex = 6;
            this.toolTip1.SetToolTip(this.comboBox_L, "单位");
            this.comboBox_L.TextChanged += new System.EventHandler(this.textBox_L_TextChanged);
            // 
            // comboBox_C
            // 
            this.comboBox_C.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox_C.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.comboBox_C.FormattingEnabled = true;
            this.comboBox_C.Items.AddRange(new object[] {
            "F",
            "mF",
            "uF",
            "nF",
            "pF"});
            this.comboBox_C.Location = new System.Drawing.Point(223, 98);
            this.comboBox_C.Name = "comboBox_C";
            this.comboBox_C.Size = new System.Drawing.Size(63, 22);
            this.comboBox_C.TabIndex = 7;
            this.toolTip1.SetToolTip(this.comboBox_C, "单位");
            this.comboBox_C.TextChanged += new System.EventHandler(this.textBox_C_TextChanged);
            // 
            // comboBox_F
            // 
            this.comboBox_F.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox_F.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.comboBox_F.ForeColor = System.Drawing.Color.Red;
            this.comboBox_F.FormattingEnabled = true;
            this.comboBox_F.Items.AddRange(new object[] {
            "Hz",
            "KHz",
            "MHz",
            "KMHz"});
            this.comboBox_F.Location = new System.Drawing.Point(223, 139);
            this.comboBox_F.Name = "comboBox_F";
            this.comboBox_F.Size = new System.Drawing.Size(63, 22);
            this.comboBox_F.TabIndex = 8;
            this.toolTip1.SetToolTip(this.comboBox_F, "单位");
            this.comboBox_F.TextChanged += new System.EventHandler(this.textBox_F_TextChanged);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.radioButton_ComputeC);
            this.groupBox1.Controls.Add(this.radioButton_ComputeL);
            this.groupBox1.Controls.Add(this.radioButton_ComputeF);
            this.groupBox1.Location = new System.Drawing.Point(19, 7);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(265, 35);
            this.groupBox1.TabIndex = 9;
            this.groupBox1.TabStop = false;
            // 
            // radioButton_ComputeC
            // 
            this.radioButton_ComputeC.AutoSize = true;
            this.radioButton_ComputeC.Location = new System.Drawing.Point(188, 13);
            this.radioButton_ComputeC.Name = "radioButton_ComputeC";
            this.radioButton_ComputeC.Size = new System.Drawing.Size(71, 16);
            this.radioButton_ComputeC.TabIndex = 2;
            this.radioButton_ComputeC.Text = "计算电容";
            this.toolTip1.SetToolTip(this.radioButton_ComputeC, "由LF计算C");
            this.radioButton_ComputeC.UseVisualStyleBackColor = true;
            this.radioButton_ComputeC.CheckedChanged += new System.EventHandler(this.radioButton_ComputeC_CheckedChanged);
            // 
            // radioButton_ComputeL
            // 
            this.radioButton_ComputeL.AutoSize = true;
            this.radioButton_ComputeL.Location = new System.Drawing.Point(97, 13);
            this.radioButton_ComputeL.Name = "radioButton_ComputeL";
            this.radioButton_ComputeL.Size = new System.Drawing.Size(71, 16);
            this.radioButton_ComputeL.TabIndex = 1;
            this.radioButton_ComputeL.Text = "计算电感";
            this.toolTip1.SetToolTip(this.radioButton_ComputeL, "由CF计算L");
            this.radioButton_ComputeL.UseVisualStyleBackColor = true;
            this.radioButton_ComputeL.CheckedChanged += new System.EventHandler(this.radioButton_ComputeL_CheckedChanged);
            // 
            // radioButton_ComputeF
            // 
            this.radioButton_ComputeF.AutoSize = true;
            this.radioButton_ComputeF.Checked = true;
            this.radioButton_ComputeF.Location = new System.Drawing.Point(6, 13);
            this.radioButton_ComputeF.Name = "radioButton_ComputeF";
            this.radioButton_ComputeF.Size = new System.Drawing.Size(71, 16);
            this.radioButton_ComputeF.TabIndex = 0;
            this.radioButton_ComputeF.TabStop = true;
            this.radioButton_ComputeF.Text = "计算频率";
            this.toolTip1.SetToolTip(this.radioButton_ComputeF, "由LC计算F");
            this.radioButton_ComputeF.UseVisualStyleBackColor = true;
            this.radioButton_ComputeF.CheckedChanged += new System.EventHandler(this.radioButton_ComputeF_CheckedChanged);
            // 
            // toolTip1
            // 
            this.toolTip1.AutomaticDelay = 100;
            // 
            // Form6
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(192)))), ((int)(((byte)(255)))));
            this.ClientSize = new System.Drawing.Size(301, 174);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.comboBox_F);
            this.Controls.Add(this.comboBox_C);
            this.Controls.Add(this.comboBox_L);
            this.Controls.Add(this.textBox_F);
            this.Controls.Add(this.textBox_C);
            this.Controls.Add(this.textBox_L);
            this.Controls.Add(this.label_F);
            this.Controls.Add(this.label_C);
            this.Controls.Add(this.label_L);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "Form6";
            this.Text = "频率计算 计Y062班 曹福昂";
            this.Load += new System.EventHandler(this.Form6_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label_L;
        private System.Windows.Forms.Label label_C;
        private System.Windows.Forms.Label label_F;
        private System.Windows.Forms.TextBox textBox_L;
        private System.Windows.Forms.TextBox textBox_C;
        private System.Windows.Forms.TextBox textBox_F;
        private System.Windows.Forms.ComboBox comboBox_L;
        private System.Windows.Forms.ComboBox comboBox_C;
        private System.Windows.Forms.ComboBox comboBox_F;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.RadioButton radioButton_ComputeC;
        private System.Windows.Forms.RadioButton radioButton_ComputeL;
        private System.Windows.Forms.RadioButton radioButton_ComputeF;
        private System.Windows.Forms.ToolTip toolTip1;
    }
}