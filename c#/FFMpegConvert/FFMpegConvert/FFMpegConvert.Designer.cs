namespace FFMpegConvert
{
    partial class FFMpegConvert
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
            this.labelVidecCodec = new System.Windows.Forms.Label();
            this.labelVideoBitRate = new System.Windows.Forms.Label();
            this.comboBoxVideoCodec = new System.Windows.Forms.ComboBox();
            this.comboBoxVideoBitRate = new System.Windows.Forms.ComboBox();
            this.comboBoxAudioBitRate = new System.Windows.Forms.ComboBox();
            this.comboBoxAudioCodec = new System.Windows.Forms.ComboBox();
            this.labelAudioBitRate = new System.Windows.Forms.Label();
            this.labelAudioCodec = new System.Windows.Forms.Label();
            this.labelInputDir = new System.Windows.Forms.Label();
            this.labelOutputDir = new System.Windows.Forms.Label();
            this.textBoxInputDir = new System.Windows.Forms.TextBox();
            this.textBoxOutputDir = new System.Windows.Forms.TextBox();
            this.folderBrowserDialogInputDir = new System.Windows.Forms.FolderBrowserDialog();
            this.folderBrowserDialogOutputDir = new System.Windows.Forms.FolderBrowserDialog();
            this.buttonStart = new System.Windows.Forms.Button();
            this.buttonStop = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // labelVidecCodec
            // 
            this.labelVidecCodec.AutoSize = true;
            this.labelVidecCodec.Location = new System.Drawing.Point(12, 73);
            this.labelVidecCodec.Name = "labelVidecCodec";
            this.labelVidecCodec.Size = new System.Drawing.Size(77, 12);
            this.labelVidecCodec.TabIndex = 0;
            this.labelVidecCodec.Text = "视频编码器：";
            // 
            // labelVideoBitRate
            // 
            this.labelVideoBitRate.AutoSize = true;
            this.labelVideoBitRate.Location = new System.Drawing.Point(12, 101);
            this.labelVideoBitRate.Name = "labelVideoBitRate";
            this.labelVideoBitRate.Size = new System.Drawing.Size(89, 12);
            this.labelVideoBitRate.TabIndex = 1;
            this.labelVideoBitRate.Text = "视频编码速率：";
            // 
            // comboBoxVideoCodec
            // 
            this.comboBoxVideoCodec.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxVideoCodec.FormattingEnabled = true;
            this.comboBoxVideoCodec.Items.AddRange(new object[] {
            "libx265",
            "libx264"});
            this.comboBoxVideoCodec.Location = new System.Drawing.Point(97, 69);
            this.comboBoxVideoCodec.Name = "comboBoxVideoCodec";
            this.comboBoxVideoCodec.Size = new System.Drawing.Size(185, 20);
            this.comboBoxVideoCodec.TabIndex = 2;
            // 
            // comboBoxVideoBitRate
            // 
            this.comboBoxVideoBitRate.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxVideoBitRate.FormattingEnabled = true;
            this.comboBoxVideoBitRate.Items.AddRange(new object[] {
            "2000k"});
            this.comboBoxVideoBitRate.Location = new System.Drawing.Point(97, 97);
            this.comboBoxVideoBitRate.Name = "comboBoxVideoBitRate";
            this.comboBoxVideoBitRate.Size = new System.Drawing.Size(185, 20);
            this.comboBoxVideoBitRate.TabIndex = 3;
            // 
            // comboBoxAudioBitRate
            // 
            this.comboBoxAudioBitRate.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxAudioBitRate.FormattingEnabled = true;
            this.comboBoxAudioBitRate.Items.AddRange(new object[] {
            "500k"});
            this.comboBoxAudioBitRate.Location = new System.Drawing.Point(97, 153);
            this.comboBoxAudioBitRate.Name = "comboBoxAudioBitRate";
            this.comboBoxAudioBitRate.Size = new System.Drawing.Size(185, 20);
            this.comboBoxAudioBitRate.TabIndex = 7;
            // 
            // comboBoxAudioCodec
            // 
            this.comboBoxAudioCodec.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxAudioCodec.FormattingEnabled = true;
            this.comboBoxAudioCodec.Items.AddRange(new object[] {
            "AC3",
            "DTS"});
            this.comboBoxAudioCodec.Location = new System.Drawing.Point(97, 125);
            this.comboBoxAudioCodec.Name = "comboBoxAudioCodec";
            this.comboBoxAudioCodec.Size = new System.Drawing.Size(185, 20);
            this.comboBoxAudioCodec.TabIndex = 6;
            // 
            // labelAudioBitRate
            // 
            this.labelAudioBitRate.AutoSize = true;
            this.labelAudioBitRate.Location = new System.Drawing.Point(12, 157);
            this.labelAudioBitRate.Name = "labelAudioBitRate";
            this.labelAudioBitRate.Size = new System.Drawing.Size(89, 12);
            this.labelAudioBitRate.TabIndex = 5;
            this.labelAudioBitRate.Text = "音频编码速率：";
            // 
            // labelAudioCodec
            // 
            this.labelAudioCodec.AutoSize = true;
            this.labelAudioCodec.Location = new System.Drawing.Point(12, 129);
            this.labelAudioCodec.Name = "labelAudioCodec";
            this.labelAudioCodec.Size = new System.Drawing.Size(77, 12);
            this.labelAudioCodec.TabIndex = 4;
            this.labelAudioCodec.Text = "音频编码器：";
            // 
            // labelInputDir
            // 
            this.labelInputDir.AutoSize = true;
            this.labelInputDir.Location = new System.Drawing.Point(12, 17);
            this.labelInputDir.Name = "labelInputDir";
            this.labelInputDir.Size = new System.Drawing.Size(77, 12);
            this.labelInputDir.TabIndex = 8;
            this.labelInputDir.Text = "输入文件夹：";
            // 
            // labelOutputDir
            // 
            this.labelOutputDir.AutoSize = true;
            this.labelOutputDir.Location = new System.Drawing.Point(12, 45);
            this.labelOutputDir.Name = "labelOutputDir";
            this.labelOutputDir.Size = new System.Drawing.Size(77, 12);
            this.labelOutputDir.TabIndex = 9;
            this.labelOutputDir.Text = "输出文件夹：";
            // 
            // textBoxInputDir
            // 
            this.textBoxInputDir.Location = new System.Drawing.Point(97, 14);
            this.textBoxInputDir.Name = "textBoxInputDir";
            this.textBoxInputDir.ReadOnly = true;
            this.textBoxInputDir.Size = new System.Drawing.Size(185, 21);
            this.textBoxInputDir.TabIndex = 10;
            this.textBoxInputDir.Click += new System.EventHandler(this.textBoxInputDir_Click);
            // 
            // textBoxOutputDir
            // 
            this.textBoxOutputDir.Location = new System.Drawing.Point(97, 41);
            this.textBoxOutputDir.Name = "textBoxOutputDir";
            this.textBoxOutputDir.ReadOnly = true;
            this.textBoxOutputDir.Size = new System.Drawing.Size(185, 21);
            this.textBoxOutputDir.TabIndex = 11;
            this.textBoxOutputDir.Click += new System.EventHandler(this.textBoxOutputDir_Click);
            // 
            // folderBrowserDialogInputDir
            // 
            this.folderBrowserDialogInputDir.Description = "选择输入文件夹";
            this.folderBrowserDialogInputDir.ShowNewFolderButton = false;
            // 
            // folderBrowserDialogOutputDir
            // 
            this.folderBrowserDialogOutputDir.Description = "选择输出文件夹";
            // 
            // buttonStart
            // 
            this.buttonStart.Enabled = false;
            this.buttonStart.Location = new System.Drawing.Point(12, 214);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(75, 23);
            this.buttonStart.TabIndex = 12;
            this.buttonStart.Text = "开始";
            this.buttonStart.UseVisualStyleBackColor = true;
            this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
            // 
            // buttonStop
            // 
            this.buttonStop.Enabled = false;
            this.buttonStop.Location = new System.Drawing.Point(205, 214);
            this.buttonStop.Name = "buttonStop";
            this.buttonStop.Size = new System.Drawing.Size(75, 23);
            this.buttonStop.TabIndex = 13;
            this.buttonStop.Text = "停止";
            this.buttonStop.UseVisualStyleBackColor = true;
            this.buttonStop.Click += new System.EventHandler(this.buttonStop_Click);
            // 
            // Convert
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(294, 258);
            this.Controls.Add(this.buttonStop);
            this.Controls.Add(this.buttonStart);
            this.Controls.Add(this.textBoxOutputDir);
            this.Controls.Add(this.textBoxInputDir);
            this.Controls.Add(this.labelOutputDir);
            this.Controls.Add(this.labelInputDir);
            this.Controls.Add(this.comboBoxAudioBitRate);
            this.Controls.Add(this.comboBoxAudioCodec);
            this.Controls.Add(this.labelAudioBitRate);
            this.Controls.Add(this.labelAudioCodec);
            this.Controls.Add(this.comboBoxVideoBitRate);
            this.Controls.Add(this.comboBoxVideoCodec);
            this.Controls.Add(this.labelVideoBitRate);
            this.Controls.Add(this.labelVidecCodec);
            this.Name = "Convert";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "视频格式转换";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label labelVidecCodec;
        private System.Windows.Forms.Label labelVideoBitRate;
        private System.Windows.Forms.ComboBox comboBoxVideoCodec;
        private System.Windows.Forms.ComboBox comboBoxVideoBitRate;
        private System.Windows.Forms.ComboBox comboBoxAudioBitRate;
        private System.Windows.Forms.ComboBox comboBoxAudioCodec;
        private System.Windows.Forms.Label labelAudioBitRate;
        private System.Windows.Forms.Label labelAudioCodec;
        private System.Windows.Forms.Label labelInputDir;
        private System.Windows.Forms.Label labelOutputDir;
        private System.Windows.Forms.TextBox textBoxInputDir;
        private System.Windows.Forms.TextBox textBoxOutputDir;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialogInputDir;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialogOutputDir;
        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.Button buttonStop;
    }
}

