namespace NetworkInputMethod
{
    partial class FormNetworkIme
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
        /// 设计器支持所需的方法 - 不要修改
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            this.labelPort = new System.Windows.Forms.Label();
            this.textBoxPort = new System.Windows.Forms.TextBox();
            this.buttonStart = new System.Windows.Forms.Button();
            this.buttonStop = new System.Windows.Forms.Button();
            this.checkedListBoxClients = new System.Windows.Forms.CheckedListBox();
            this.textBoxContent = new System.Windows.Forms.TextBox();
            this.buttonSend = new System.Windows.Forms.Button();
            this.buttonClear = new System.Windows.Forms.Button();
            this.groupBoxMode = new System.Windows.Forms.GroupBox();
            this.radioButtonSend = new System.Windows.Forms.RadioButton();
            this.radioButtonReplace = new System.Windows.Forms.RadioButton();
            this.radioButtonInsert = new System.Windows.Forms.RadioButton();
            this.checkBoxClear = new System.Windows.Forms.CheckBox();
            this.groupBoxMode.SuspendLayout();
            this.SuspendLayout();
            // 
            // labelPort
            // 
            this.labelPort.AutoSize = true;
            this.labelPort.Location = new System.Drawing.Point(12, 15);
            this.labelPort.Name = "labelPort";
            this.labelPort.Size = new System.Drawing.Size(53, 12);
            this.labelPort.TabIndex = 0;
            this.labelPort.Text = "端口号：";
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(71, 12);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(385, 21);
            this.textBoxPort.TabIndex = 1;
            this.textBoxPort.Text = "8865";
            // 
            // buttonStart
            // 
            this.buttonStart.Location = new System.Drawing.Point(14, 456);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(75, 23);
            this.buttonStart.TabIndex = 2;
            this.buttonStart.Text = "启动";
            this.buttonStart.UseVisualStyleBackColor = true;
            this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
            // 
            // buttonStop
            // 
            this.buttonStop.Location = new System.Drawing.Point(95, 456);
            this.buttonStop.Name = "buttonStop";
            this.buttonStop.Size = new System.Drawing.Size(75, 23);
            this.buttonStop.TabIndex = 3;
            this.buttonStop.Text = "停止";
            this.buttonStop.UseVisualStyleBackColor = true;
            this.buttonStop.Click += new System.EventHandler(this.buttonStop_Click);
            // 
            // checkedListBoxClients
            // 
            this.checkedListBoxClients.CheckOnClick = true;
            this.checkedListBoxClients.FormattingEnabled = true;
            this.checkedListBoxClients.Location = new System.Drawing.Point(14, 39);
            this.checkedListBoxClients.Name = "checkedListBoxClients";
            this.checkedListBoxClients.Size = new System.Drawing.Size(442, 132);
            this.checkedListBoxClients.TabIndex = 4;
            // 
            // textBoxContent
            // 
            this.textBoxContent.Location = new System.Drawing.Point(14, 177);
            this.textBoxContent.Multiline = true;
            this.textBoxContent.Name = "textBoxContent";
            this.textBoxContent.Size = new System.Drawing.Size(442, 219);
            this.textBoxContent.TabIndex = 5;
            // 
            // buttonSend
            // 
            this.buttonSend.Location = new System.Drawing.Point(381, 456);
            this.buttonSend.Name = "buttonSend";
            this.buttonSend.Size = new System.Drawing.Size(75, 23);
            this.buttonSend.TabIndex = 6;
            this.buttonSend.Text = "发送";
            this.buttonSend.UseVisualStyleBackColor = true;
            this.buttonSend.Click += new System.EventHandler(this.buttonSend_Click);
            // 
            // buttonClear
            // 
            this.buttonClear.Location = new System.Drawing.Point(176, 456);
            this.buttonClear.Name = "buttonClear";
            this.buttonClear.Size = new System.Drawing.Size(75, 23);
            this.buttonClear.TabIndex = 7;
            this.buttonClear.Text = "清除";
            this.buttonClear.UseVisualStyleBackColor = true;
            this.buttonClear.Click += new System.EventHandler(this.buttonClear_Click);
            // 
            // groupBoxMode
            // 
            this.groupBoxMode.Controls.Add(this.radioButtonReplace);
            this.groupBoxMode.Controls.Add(this.radioButtonInsert);
            this.groupBoxMode.Controls.Add(this.radioButtonSend);
            this.groupBoxMode.Location = new System.Drawing.Point(257, 402);
            this.groupBoxMode.Name = "groupBoxMode";
            this.groupBoxMode.Size = new System.Drawing.Size(199, 48);
            this.groupBoxMode.TabIndex = 8;
            this.groupBoxMode.TabStop = false;
            // 
            // radioButtonSend
            // 
            this.radioButtonSend.AutoSize = true;
            this.radioButtonSend.Checked = true;
            this.radioButtonSend.Location = new System.Drawing.Point(146, 20);
            this.radioButtonSend.Name = "radioButtonSend";
            this.radioButtonSend.Size = new System.Drawing.Size(47, 16);
            this.radioButtonSend.TabIndex = 2;
            this.radioButtonSend.TabStop = true;
            this.radioButtonSend.Text = "发送";
            this.radioButtonSend.UseVisualStyleBackColor = true;
            // 
            // radioButtonReplace
            // 
            this.radioButtonReplace.AutoSize = true;
            this.radioButtonReplace.Location = new System.Drawing.Point(73, 20);
            this.radioButtonReplace.Name = "radioButtonReplace";
            this.radioButtonReplace.Size = new System.Drawing.Size(47, 16);
            this.radioButtonReplace.TabIndex = 1;
            this.radioButtonReplace.Text = "替换";
            this.radioButtonReplace.UseVisualStyleBackColor = true;
            // 
            // radioButtonInsert
            // 
            this.radioButtonInsert.AutoSize = true;
            this.radioButtonInsert.Location = new System.Drawing.Point(6, 20);
            this.radioButtonInsert.Name = "radioButtonInsert";
            this.radioButtonInsert.Size = new System.Drawing.Size(47, 16);
            this.radioButtonInsert.TabIndex = 0;
            this.radioButtonInsert.Text = "插入";
            this.radioButtonInsert.UseVisualStyleBackColor = true;
            // 
            // checkBoxClear
            // 
            this.checkBoxClear.AutoSize = true;
            this.checkBoxClear.Location = new System.Drawing.Point(14, 402);
            this.checkBoxClear.Name = "checkBoxClear";
            this.checkBoxClear.Size = new System.Drawing.Size(84, 16);
            this.checkBoxClear.TabIndex = 9;
            this.checkBoxClear.Text = "发送后清除";
            this.checkBoxClear.UseVisualStyleBackColor = true;
            // 
            // FormNetworkIme
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(468, 491);
            this.Controls.Add(this.checkBoxClear);
            this.Controls.Add(this.groupBoxMode);
            this.Controls.Add(this.buttonClear);
            this.Controls.Add(this.buttonSend);
            this.Controls.Add(this.textBoxContent);
            this.Controls.Add(this.checkedListBoxClients);
            this.Controls.Add(this.buttonStop);
            this.Controls.Add(this.buttonStart);
            this.Controls.Add(this.textBoxPort);
            this.Controls.Add(this.labelPort);
            this.Name = "FormNetworkIme";
            this.Text = "网络输入法";
            this.groupBoxMode.ResumeLayout(false);
            this.groupBoxMode.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label labelPort;
        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.Button buttonStop;
        private System.Windows.Forms.CheckedListBox checkedListBoxClients;
        private System.Windows.Forms.TextBox textBoxContent;
        private System.Windows.Forms.Button buttonSend;
        private System.Windows.Forms.Button buttonClear;
        private System.Windows.Forms.GroupBox groupBoxMode;
        private System.Windows.Forms.RadioButton radioButtonReplace;
        private System.Windows.Forms.RadioButton radioButtonInsert;
        private System.Windows.Forms.RadioButton radioButtonSend;
        private System.Windows.Forms.CheckBox checkBoxClear;
    }
}

