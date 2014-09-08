namespace CFA090307A
{
    partial class Form2
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form2));
            this.label_binary = new System.Windows.Forms.Label();
            this.label_octal = new System.Windows.Forms.Label();
            this.label_decimal = new System.Windows.Forms.Label();
            this.label_hex = new System.Windows.Forms.Label();
            this.textBox_binary = new System.Windows.Forms.TextBox();
            this.textBox_octal = new System.Windows.Forms.TextBox();
            this.textBox_decimal = new System.Windows.Forms.TextBox();
            this.textBox_hex = new System.Windows.Forms.TextBox();
            this.button_SHL = new System.Windows.Forms.Button();
            this.button_SHR = new System.Windows.Forms.Button();
            this.button_SAL = new System.Windows.Forms.Button();
            this.button_SAR = new System.Windows.Forms.Button();
            this.button_ROL = new System.Windows.Forms.Button();
            this.button_ROR = new System.Windows.Forms.Button();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.SuspendLayout();
            // 
            // label_binary
            // 
            this.label_binary.AutoSize = true;
            this.label_binary.Location = new System.Drawing.Point(12, 19);
            this.label_binary.Name = "label_binary";
            this.label_binary.Size = new System.Drawing.Size(59, 12);
            this.label_binary.TabIndex = 1;
            this.label_binary.Text = "二 进 制:";
            // 
            // label_octal
            // 
            this.label_octal.AutoSize = true;
            this.label_octal.Location = new System.Drawing.Point(12, 48);
            this.label_octal.Name = "label_octal";
            this.label_octal.Size = new System.Drawing.Size(59, 12);
            this.label_octal.TabIndex = 3;
            this.label_octal.Text = "八 进 制:";
            // 
            // label_decimal
            // 
            this.label_decimal.AutoSize = true;
            this.label_decimal.Location = new System.Drawing.Point(12, 77);
            this.label_decimal.Name = "label_decimal";
            this.label_decimal.Size = new System.Drawing.Size(59, 12);
            this.label_decimal.TabIndex = 7;
            this.label_decimal.Text = "十 进 制:";
            // 
            // label_hex
            // 
            this.label_hex.AutoSize = true;
            this.label_hex.Location = new System.Drawing.Point(12, 106);
            this.label_hex.Name = "label_hex";
            this.label_hex.Size = new System.Drawing.Size(59, 12);
            this.label_hex.TabIndex = 8;
            this.label_hex.Text = "十六进制:";
            // 
            // textBox_binary
            // 
            this.textBox_binary.Location = new System.Drawing.Point(77, 12);
            this.textBox_binary.Name = "textBox_binary";
            this.textBox_binary.Size = new System.Drawing.Size(195, 21);
            this.textBox_binary.TabIndex = 9;
            this.toolTip1.SetToolTip(this.textBox_binary, "可以是小数\r\n不能是负数");
            this.textBox_binary.TextChanged += new System.EventHandler(this.textBox_binary_TextChanged);
            // 
            // textBox_octal
            // 
            this.textBox_octal.Location = new System.Drawing.Point(77, 43);
            this.textBox_octal.Name = "textBox_octal";
            this.textBox_octal.Size = new System.Drawing.Size(195, 21);
            this.textBox_octal.TabIndex = 10;
            this.toolTip1.SetToolTip(this.textBox_octal, "可以是小数\r\n不能是负数");
            this.textBox_octal.TextChanged += new System.EventHandler(this.textBox_octal_TextChanged);
            // 
            // textBox_decimal
            // 
            this.textBox_decimal.Location = new System.Drawing.Point(77, 74);
            this.textBox_decimal.Name = "textBox_decimal";
            this.textBox_decimal.Size = new System.Drawing.Size(195, 21);
            this.textBox_decimal.TabIndex = 0;
            this.toolTip1.SetToolTip(this.textBox_decimal, "可以是一个算式，但\r\n算式的值不能是负数");
            this.textBox_decimal.TextChanged += new System.EventHandler(this.textBox_decimal_TextChanged);
            // 
            // textBox_hex
            // 
            this.textBox_hex.Location = new System.Drawing.Point(77, 105);
            this.textBox_hex.Name = "textBox_hex";
            this.textBox_hex.Size = new System.Drawing.Size(195, 21);
            this.textBox_hex.TabIndex = 12;
            this.toolTip1.SetToolTip(this.textBox_hex, "可以是小数\r\n不能是负数");
            this.textBox_hex.TextChanged += new System.EventHandler(this.textBox_hex_TextChanged);
            // 
            // button_SHL
            // 
            this.button_SHL.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.button_SHL.Location = new System.Drawing.Point(292, 12);
            this.button_SHL.Name = "button_SHL";
            this.button_SHL.Size = new System.Drawing.Size(75, 34);
            this.button_SHL.TabIndex = 13;
            this.button_SHL.Text = "逻辑左移";
            this.button_SHL.UseVisualStyleBackColor = true;
            this.button_SHL.Click += new System.EventHandler(this.button_SHL_Click);
            // 
            // button_SHR
            // 
            this.button_SHR.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.button_SHR.Location = new System.Drawing.Point(373, 12);
            this.button_SHR.Name = "button_SHR";
            this.button_SHR.Size = new System.Drawing.Size(75, 34);
            this.button_SHR.TabIndex = 14;
            this.button_SHR.Text = "逻辑右移";
            this.button_SHR.UseVisualStyleBackColor = true;
            this.button_SHR.Click += new System.EventHandler(this.button_SHR_Click);
            // 
            // button_SAL
            // 
            this.button_SAL.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.button_SAL.Location = new System.Drawing.Point(292, 52);
            this.button_SAL.Name = "button_SAL";
            this.button_SAL.Size = new System.Drawing.Size(75, 34);
            this.button_SAL.TabIndex = 15;
            this.button_SAL.Text = "算术左移";
            this.button_SAL.UseVisualStyleBackColor = true;
            this.button_SAL.Click += new System.EventHandler(this.button_SHL_Click);
            // 
            // button_SAR
            // 
            this.button_SAR.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.button_SAR.Location = new System.Drawing.Point(373, 52);
            this.button_SAR.Name = "button_SAR";
            this.button_SAR.Size = new System.Drawing.Size(75, 34);
            this.button_SAR.TabIndex = 16;
            this.button_SAR.Text = "算术右移";
            this.button_SAR.UseVisualStyleBackColor = true;
            this.button_SAR.Click += new System.EventHandler(this.button_SAR_Click);
            // 
            // button_ROL
            // 
            this.button_ROL.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.button_ROL.Location = new System.Drawing.Point(292, 92);
            this.button_ROL.Name = "button_ROL";
            this.button_ROL.Size = new System.Drawing.Size(75, 34);
            this.button_ROL.TabIndex = 17;
            this.button_ROL.Text = "循环左移";
            this.button_ROL.UseVisualStyleBackColor = true;
            this.button_ROL.Click += new System.EventHandler(this.button_ROL_Click);
            // 
            // button_ROR
            // 
            this.button_ROR.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.button_ROR.Location = new System.Drawing.Point(373, 92);
            this.button_ROR.Name = "button_ROR";
            this.button_ROR.Size = new System.Drawing.Size(75, 34);
            this.button_ROR.TabIndex = 18;
            this.button_ROR.Text = "循环右移";
            this.button_ROR.UseVisualStyleBackColor = true;
            this.button_ROR.Click += new System.EventHandler(this.button_ROR_Click);
            // 
            // toolTip1
            // 
            this.toolTip1.AutomaticDelay = 100;
            // 
            // Form2
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.MistyRose;
            this.ClientSize = new System.Drawing.Size(456, 137);
            this.Controls.Add(this.button_ROR);
            this.Controls.Add(this.button_ROL);
            this.Controls.Add(this.button_SAR);
            this.Controls.Add(this.button_SAL);
            this.Controls.Add(this.button_SHR);
            this.Controls.Add(this.button_SHL);
            this.Controls.Add(this.textBox_hex);
            this.Controls.Add(this.textBox_decimal);
            this.Controls.Add(this.textBox_octal);
            this.Controls.Add(this.textBox_binary);
            this.Controls.Add(this.label_hex);
            this.Controls.Add(this.label_decimal);
            this.Controls.Add(this.label_octal);
            this.Controls.Add(this.label_binary);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "Form2";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "进制间的相互转换  计Y062班 曹福昂";
            this.Load += new System.EventHandler(this.Form2_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label_binary;
        private System.Windows.Forms.Label label_octal;
        private System.Windows.Forms.Label label_decimal;
        private System.Windows.Forms.Label label_hex;
        private System.Windows.Forms.TextBox textBox_binary;
        private System.Windows.Forms.TextBox textBox_octal;
        private System.Windows.Forms.TextBox textBox_hex;
        private System.Windows.Forms.Button button_SHL;
        private System.Windows.Forms.Button button_SHR;
        private System.Windows.Forms.Button button_SAL;
        private System.Windows.Forms.Button button_SAR;
        private System.Windows.Forms.Button button_ROL;
        private System.Windows.Forms.Button button_ROR;
        private System.Windows.Forms.ToolTip toolTip1;
        public System.Windows.Forms.TextBox textBox_decimal;
    }
}