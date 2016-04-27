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
            this.labelInDir = new System.Windows.Forms.Label();
            this.labelOutDir = new System.Windows.Forms.Label();
            this.textBoxInDir = new System.Windows.Forms.TextBox();
            this.textBoxOutDir = new System.Windows.Forms.TextBox();
            this.folderBrowserDialogInDir = new System.Windows.Forms.FolderBrowserDialog();
            this.folderBrowserDialogOutDir = new System.Windows.Forms.FolderBrowserDialog();
            this.buttonStart = new System.Windows.Forms.Button();
            this.buttonStop = new System.Windows.Forms.Button();
            this.labelVideoCodecParam = new System.Windows.Forms.Label();
            this.textBoxVidecCodecParam = new System.Windows.Forms.TextBox();
            this.checkBoxOverride = new System.Windows.Forms.CheckBox();
            this.labelLogFile = new System.Windows.Forms.Label();
            this.textBoxLogFile = new System.Windows.Forms.TextBox();
            this.saveFileDialogLogFile = new System.Windows.Forms.SaveFileDialog();
            this.SuspendLayout();
            // 
            // labelVidecCodec
            // 
            this.labelVidecCodec.AutoSize = true;
            this.labelVidecCodec.Location = new System.Drawing.Point(12, 71);
            this.labelVidecCodec.Name = "labelVidecCodec";
            this.labelVidecCodec.Size = new System.Drawing.Size(77, 12);
            this.labelVidecCodec.TabIndex = 0;
            this.labelVidecCodec.Text = "视频编码器：";
            // 
            // labelVideoBitRate
            // 
            this.labelVideoBitRate.AutoSize = true;
            this.labelVideoBitRate.Location = new System.Drawing.Point(12, 98);
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
            this.comboBoxVideoCodec.Location = new System.Drawing.Point(103, 67);
            this.comboBoxVideoCodec.Name = "comboBoxVideoCodec";
            this.comboBoxVideoCodec.Size = new System.Drawing.Size(266, 20);
            this.comboBoxVideoCodec.TabIndex = 2;
            // 
            // comboBoxVideoBitRate
            // 
            this.comboBoxVideoBitRate.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxVideoBitRate.FormattingEnabled = true;
            this.comboBoxVideoBitRate.Items.AddRange(new object[] {
            "2000k"});
            this.comboBoxVideoBitRate.Location = new System.Drawing.Point(103, 94);
            this.comboBoxVideoBitRate.Name = "comboBoxVideoBitRate";
            this.comboBoxVideoBitRate.Size = new System.Drawing.Size(266, 20);
            this.comboBoxVideoBitRate.TabIndex = 3;
            // 
            // comboBoxAudioBitRate
            // 
            this.comboBoxAudioBitRate.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxAudioBitRate.FormattingEnabled = true;
            this.comboBoxAudioBitRate.Items.AddRange(new object[] {
            "500k"});
            this.comboBoxAudioBitRate.Location = new System.Drawing.Point(103, 175);
            this.comboBoxAudioBitRate.Name = "comboBoxAudioBitRate";
            this.comboBoxAudioBitRate.Size = new System.Drawing.Size(266, 20);
            this.comboBoxAudioBitRate.TabIndex = 7;
            // 
            // comboBoxAudioCodec
            // 
            this.comboBoxAudioCodec.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxAudioCodec.FormattingEnabled = true;
            this.comboBoxAudioCodec.Items.AddRange(new object[] {
            "ac3"});
            this.comboBoxAudioCodec.Location = new System.Drawing.Point(103, 148);
            this.comboBoxAudioCodec.Name = "comboBoxAudioCodec";
            this.comboBoxAudioCodec.Size = new System.Drawing.Size(266, 20);
            this.comboBoxAudioCodec.TabIndex = 6;
            // 
            // labelAudioBitRate
            // 
            this.labelAudioBitRate.AutoSize = true;
            this.labelAudioBitRate.Location = new System.Drawing.Point(12, 179);
            this.labelAudioBitRate.Name = "labelAudioBitRate";
            this.labelAudioBitRate.Size = new System.Drawing.Size(89, 12);
            this.labelAudioBitRate.TabIndex = 5;
            this.labelAudioBitRate.Text = "音频编码速率：";
            // 
            // labelAudioCodec
            // 
            this.labelAudioCodec.AutoSize = true;
            this.labelAudioCodec.Location = new System.Drawing.Point(12, 152);
            this.labelAudioCodec.Name = "labelAudioCodec";
            this.labelAudioCodec.Size = new System.Drawing.Size(77, 12);
            this.labelAudioCodec.TabIndex = 4;
            this.labelAudioCodec.Text = "音频编码器：";
            // 
            // labelInDir
            // 
            this.labelInDir.AutoSize = true;
            this.labelInDir.Location = new System.Drawing.Point(12, 17);
            this.labelInDir.Name = "labelInDir";
            this.labelInDir.Size = new System.Drawing.Size(77, 12);
            this.labelInDir.TabIndex = 8;
            this.labelInDir.Text = "输入文件夹：";
            // 
            // labelOutDir
            // 
            this.labelOutDir.AutoSize = true;
            this.labelOutDir.Location = new System.Drawing.Point(12, 44);
            this.labelOutDir.Name = "labelOutDir";
            this.labelOutDir.Size = new System.Drawing.Size(77, 12);
            this.labelOutDir.TabIndex = 9;
            this.labelOutDir.Text = "输出文件夹：";
            // 
            // textBoxInDir
            // 
            this.textBoxInDir.Location = new System.Drawing.Point(103, 13);
            this.textBoxInDir.Name = "textBoxInDir";
            this.textBoxInDir.ReadOnly = true;
            this.textBoxInDir.Size = new System.Drawing.Size(266, 21);
            this.textBoxInDir.TabIndex = 10;
            this.textBoxInDir.Click += new System.EventHandler(this.textBoxInputDir_Click);
            // 
            // textBoxOutDir
            // 
            this.textBoxOutDir.Location = new System.Drawing.Point(103, 40);
            this.textBoxOutDir.Name = "textBoxOutDir";
            this.textBoxOutDir.ReadOnly = true;
            this.textBoxOutDir.Size = new System.Drawing.Size(266, 21);
            this.textBoxOutDir.TabIndex = 11;
            this.textBoxOutDir.Click += new System.EventHandler(this.textBoxOutputDir_Click);
            // 
            // folderBrowserDialogInDir
            // 
            this.folderBrowserDialogInDir.Description = "选择输入文件夹";
            this.folderBrowserDialogInDir.ShowNewFolderButton = false;
            // 
            // folderBrowserDialogOutDir
            // 
            this.folderBrowserDialogOutDir.Description = "选择输出文件夹";
            // 
            // buttonStart
            // 
            this.buttonStart.Enabled = false;
            this.buttonStart.Location = new System.Drawing.Point(14, 280);
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
            this.buttonStop.Location = new System.Drawing.Point(294, 280);
            this.buttonStop.Name = "buttonStop";
            this.buttonStop.Size = new System.Drawing.Size(75, 23);
            this.buttonStop.TabIndex = 13;
            this.buttonStop.Text = "停止";
            this.buttonStop.UseVisualStyleBackColor = true;
            this.buttonStop.Click += new System.EventHandler(this.buttonStop_Click);
            // 
            // labelVideoCodecParam
            // 
            this.labelVideoCodecParam.AutoSize = true;
            this.labelVideoCodecParam.Location = new System.Drawing.Point(12, 125);
            this.labelVideoCodecParam.Name = "labelVideoCodecParam";
            this.labelVideoCodecParam.Size = new System.Drawing.Size(89, 12);
            this.labelVideoCodecParam.TabIndex = 14;
            this.labelVideoCodecParam.Text = "视频编码参数：";
            // 
            // textBoxVidecCodecParam
            // 
            this.textBoxVidecCodecParam.Location = new System.Drawing.Point(103, 121);
            this.textBoxVidecCodecParam.Name = "textBoxVidecCodecParam";
            this.textBoxVidecCodecParam.Size = new System.Drawing.Size(266, 21);
            this.textBoxVidecCodecParam.TabIndex = 15;
            this.textBoxVidecCodecParam.Text = "-preset fast";
            // 
            // checkBoxOverride
            // 
            this.checkBoxOverride.AutoSize = true;
            this.checkBoxOverride.Location = new System.Drawing.Point(12, 234);
            this.checkBoxOverride.Name = "checkBoxOverride";
            this.checkBoxOverride.Size = new System.Drawing.Size(120, 16);
            this.checkBoxOverride.TabIndex = 16;
            this.checkBoxOverride.Text = "覆盖已存在的文件";
            this.checkBoxOverride.UseVisualStyleBackColor = true;
            // 
            // labelLogFile
            // 
            this.labelLogFile.AutoSize = true;
            this.labelLogFile.Location = new System.Drawing.Point(12, 206);
            this.labelLogFile.Name = "labelLogFile";
            this.labelLogFile.Size = new System.Drawing.Size(65, 12);
            this.labelLogFile.TabIndex = 17;
            this.labelLogFile.Text = "日志文件：";
            // 
            // textBoxLogFile
            // 
            this.textBoxLogFile.Location = new System.Drawing.Point(103, 202);
            this.textBoxLogFile.Name = "textBoxLogFile";
            this.textBoxLogFile.ReadOnly = true;
            this.textBoxLogFile.Size = new System.Drawing.Size(266, 21);
            this.textBoxLogFile.TabIndex = 18;
            this.textBoxLogFile.Click += new System.EventHandler(this.textBoxLogFile_Click);
            // 
            // saveFileDialogLogFile
            // 
            this.saveFileDialogLogFile.FileName = "ffmpeg-convert.log";
            this.saveFileDialogLogFile.Title = "选择日志文件";
            // 
            // FFMpegConvert
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(381, 314);
            this.Controls.Add(this.textBoxLogFile);
            this.Controls.Add(this.labelLogFile);
            this.Controls.Add(this.checkBoxOverride);
            this.Controls.Add(this.textBoxVidecCodecParam);
            this.Controls.Add(this.labelVideoCodecParam);
            this.Controls.Add(this.buttonStop);
            this.Controls.Add(this.buttonStart);
            this.Controls.Add(this.textBoxOutDir);
            this.Controls.Add(this.textBoxInDir);
            this.Controls.Add(this.labelOutDir);
            this.Controls.Add(this.labelInDir);
            this.Controls.Add(this.comboBoxAudioBitRate);
            this.Controls.Add(this.comboBoxAudioCodec);
            this.Controls.Add(this.labelAudioBitRate);
            this.Controls.Add(this.labelAudioCodec);
            this.Controls.Add(this.comboBoxVideoBitRate);
            this.Controls.Add(this.comboBoxVideoCodec);
            this.Controls.Add(this.labelVideoBitRate);
            this.Controls.Add(this.labelVidecCodec);
            this.Name = "FFMpegConvert";
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
        private System.Windows.Forms.Label labelInDir;
        private System.Windows.Forms.Label labelOutDir;
        private System.Windows.Forms.TextBox textBoxInDir;
        private System.Windows.Forms.TextBox textBoxOutDir;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialogInDir;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialogOutDir;
        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.Button buttonStop;
        private System.Windows.Forms.Label labelVideoCodecParam;
        private System.Windows.Forms.TextBox textBoxVidecCodecParam;
        private System.Windows.Forms.CheckBox checkBoxOverride;
        private System.Windows.Forms.Label labelLogFile;
        private System.Windows.Forms.TextBox textBoxLogFile;
        private System.Windows.Forms.SaveFileDialog saveFileDialogLogFile;
    }
}

