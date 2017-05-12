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
            this.textBoxInPath = new System.Windows.Forms.TextBox();
            this.textBoxOutPath = new System.Windows.Forms.TextBox();
            this.folderBrowserDialogInDir = new System.Windows.Forms.FolderBrowserDialog();
            this.folderBrowserDialogOutDir = new System.Windows.Forms.FolderBrowserDialog();
            this.buttonStart = new System.Windows.Forms.Button();
            this.buttonStop = new System.Windows.Forms.Button();
            this.labelVideoCodecParam = new System.Windows.Forms.Label();
            this.textBoxParam = new System.Windows.Forms.TextBox();
            this.checkBoxOverride = new System.Windows.Forms.CheckBox();
            this.labelLogFile = new System.Windows.Forms.Label();
            this.textBoxLogFile = new System.Windows.Forms.TextBox();
            this.saveFileDialogLogFile = new System.Windows.Forms.SaveFileDialog();
            this.textBoxLog = new System.Windows.Forms.TextBox();
            this.buttonClear = new System.Windows.Forms.Button();
            this.labelState = new System.Windows.Forms.Label();
            this.checkBoxHiddenCmdline = new System.Windows.Forms.CheckBox();
            this.label1 = new System.Windows.Forms.Label();
            this.comboBoxOutputFormat = new System.Windows.Forms.ComboBox();
            this.buttonInFile = new System.Windows.Forms.Button();
            this.buttonInDir = new System.Windows.Forms.Button();
            this.buttonOutDir = new System.Windows.Forms.Button();
            this.openFileDialogInFile = new System.Windows.Forms.OpenFileDialog();
            this.backgroundWorkerConvert = new System.ComponentModel.BackgroundWorker();
            this.SuspendLayout();
            // 
            // labelVidecCodec
            // 
            this.labelVidecCodec.AutoSize = true;
            this.labelVidecCodec.Location = new System.Drawing.Point(3, 379);
            this.labelVidecCodec.Name = "labelVidecCodec";
            this.labelVidecCodec.Size = new System.Drawing.Size(77, 12);
            this.labelVidecCodec.TabIndex = 0;
            this.labelVidecCodec.Text = "视频编码器：";
            // 
            // labelVideoBitRate
            // 
            this.labelVideoBitRate.AutoSize = true;
            this.labelVideoBitRate.Location = new System.Drawing.Point(248, 379);
            this.labelVideoBitRate.Name = "labelVideoBitRate";
            this.labelVideoBitRate.Size = new System.Drawing.Size(65, 12);
            this.labelVideoBitRate.TabIndex = 1;
            this.labelVideoBitRate.Text = "视频码率：";
            // 
            // comboBoxVideoCodec
            // 
            this.comboBoxVideoCodec.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxVideoCodec.FormattingEnabled = true;
            this.comboBoxVideoCodec.Items.AddRange(new object[] {
            "libx264",
            "libx265"});
            this.comboBoxVideoCodec.Location = new System.Drawing.Point(86, 375);
            this.comboBoxVideoCodec.Name = "comboBoxVideoCodec";
            this.comboBoxVideoCodec.Size = new System.Drawing.Size(114, 20);
            this.comboBoxVideoCodec.TabIndex = 2;
            // 
            // comboBoxVideoBitRate
            // 
            this.comboBoxVideoBitRate.FormattingEnabled = true;
            this.comboBoxVideoBitRate.Items.AddRange(new object[] {
            "8000k",
            "2000k"});
            this.comboBoxVideoBitRate.Location = new System.Drawing.Point(319, 375);
            this.comboBoxVideoBitRate.Name = "comboBoxVideoBitRate";
            this.comboBoxVideoBitRate.Size = new System.Drawing.Size(261, 20);
            this.comboBoxVideoBitRate.TabIndex = 3;
            // 
            // comboBoxAudioBitRate
            // 
            this.comboBoxAudioBitRate.FormattingEnabled = true;
            this.comboBoxAudioBitRate.Items.AddRange(new object[] {
            "无",
            "500k"});
            this.comboBoxAudioBitRate.Location = new System.Drawing.Point(319, 403);
            this.comboBoxAudioBitRate.Name = "comboBoxAudioBitRate";
            this.comboBoxAudioBitRate.Size = new System.Drawing.Size(261, 20);
            this.comboBoxAudioBitRate.TabIndex = 7;
            // 
            // comboBoxAudioCodec
            // 
            this.comboBoxAudioCodec.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxAudioCodec.FormattingEnabled = true;
            this.comboBoxAudioCodec.Items.AddRange(new object[] {
            "无",
            "ac3"});
            this.comboBoxAudioCodec.Location = new System.Drawing.Point(86, 403);
            this.comboBoxAudioCodec.Name = "comboBoxAudioCodec";
            this.comboBoxAudioCodec.Size = new System.Drawing.Size(114, 20);
            this.comboBoxAudioCodec.TabIndex = 6;
            // 
            // labelAudioBitRate
            // 
            this.labelAudioBitRate.AutoSize = true;
            this.labelAudioBitRate.Location = new System.Drawing.Point(248, 407);
            this.labelAudioBitRate.Name = "labelAudioBitRate";
            this.labelAudioBitRate.Size = new System.Drawing.Size(65, 12);
            this.labelAudioBitRate.TabIndex = 5;
            this.labelAudioBitRate.Text = "音频码率：";
            // 
            // labelAudioCodec
            // 
            this.labelAudioCodec.AutoSize = true;
            this.labelAudioCodec.Location = new System.Drawing.Point(3, 407);
            this.labelAudioCodec.Name = "labelAudioCodec";
            this.labelAudioCodec.Size = new System.Drawing.Size(77, 12);
            this.labelAudioCodec.TabIndex = 4;
            this.labelAudioCodec.Text = "音频编码器：";
            // 
            // labelInDir
            // 
            this.labelInDir.AutoSize = true;
            this.labelInDir.Location = new System.Drawing.Point(3, 289);
            this.labelInDir.Name = "labelInDir";
            this.labelInDir.Size = new System.Drawing.Size(65, 12);
            this.labelInDir.TabIndex = 8;
            this.labelInDir.Text = "输入路径：";
            // 
            // labelOutDir
            // 
            this.labelOutDir.AutoSize = true;
            this.labelOutDir.Location = new System.Drawing.Point(3, 320);
            this.labelOutDir.Name = "labelOutDir";
            this.labelOutDir.Size = new System.Drawing.Size(65, 12);
            this.labelOutDir.TabIndex = 9;
            this.labelOutDir.Text = "输出路径：";
            // 
            // textBoxInPath
            // 
            this.textBoxInPath.AllowDrop = true;
            this.textBoxInPath.Location = new System.Drawing.Point(74, 285);
            this.textBoxInPath.Name = "textBoxInPath";
            this.textBoxInPath.ReadOnly = true;
            this.textBoxInPath.Size = new System.Drawing.Size(414, 21);
            this.textBoxInPath.TabIndex = 10;
            // 
            // textBoxOutPath
            // 
            this.textBoxOutPath.AllowDrop = true;
            this.textBoxOutPath.Location = new System.Drawing.Point(74, 316);
            this.textBoxOutPath.Name = "textBoxOutPath";
            this.textBoxOutPath.ReadOnly = true;
            this.textBoxOutPath.Size = new System.Drawing.Size(460, 21);
            this.textBoxOutPath.TabIndex = 11;
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
            this.buttonStart.Location = new System.Drawing.Point(475, 468);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(105, 64);
            this.buttonStart.TabIndex = 12;
            this.buttonStart.Text = "开始";
            this.buttonStart.UseVisualStyleBackColor = true;
            this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
            // 
            // buttonStop
            // 
            this.buttonStop.Enabled = false;
            this.buttonStop.Location = new System.Drawing.Point(239, 468);
            this.buttonStop.Name = "buttonStop";
            this.buttonStop.Size = new System.Drawing.Size(105, 64);
            this.buttonStop.TabIndex = 13;
            this.buttonStop.Text = "停止";
            this.buttonStop.UseVisualStyleBackColor = true;
            this.buttonStop.Click += new System.EventHandler(this.buttonStop_Click);
            // 
            // labelVideoCodecParam
            // 
            this.labelVideoCodecParam.AutoSize = true;
            this.labelVideoCodecParam.Location = new System.Drawing.Point(248, 435);
            this.labelVideoCodecParam.Name = "labelVideoCodecParam";
            this.labelVideoCodecParam.Size = new System.Drawing.Size(65, 12);
            this.labelVideoCodecParam.TabIndex = 14;
            this.labelVideoCodecParam.Text = "编码参数：";
            // 
            // textBoxParam
            // 
            this.textBoxParam.Location = new System.Drawing.Point(319, 431);
            this.textBoxParam.Name = "textBoxParam";
            this.textBoxParam.Size = new System.Drawing.Size(261, 21);
            this.textBoxParam.TabIndex = 15;
            // 
            // checkBoxOverride
            // 
            this.checkBoxOverride.AutoSize = true;
            this.checkBoxOverride.Location = new System.Drawing.Point(3, 260);
            this.checkBoxOverride.Name = "checkBoxOverride";
            this.checkBoxOverride.Size = new System.Drawing.Size(120, 16);
            this.checkBoxOverride.TabIndex = 16;
            this.checkBoxOverride.Text = "覆盖已存在的文件";
            this.checkBoxOverride.UseVisualStyleBackColor = true;
            // 
            // labelLogFile
            // 
            this.labelLogFile.AutoSize = true;
            this.labelLogFile.Location = new System.Drawing.Point(3, 350);
            this.labelLogFile.Name = "labelLogFile";
            this.labelLogFile.Size = new System.Drawing.Size(65, 12);
            this.labelLogFile.TabIndex = 17;
            this.labelLogFile.Text = "日志文件：";
            // 
            // textBoxLogFile
            // 
            this.textBoxLogFile.Location = new System.Drawing.Point(74, 346);
            this.textBoxLogFile.Name = "textBoxLogFile";
            this.textBoxLogFile.ReadOnly = true;
            this.textBoxLogFile.Size = new System.Drawing.Size(506, 21);
            this.textBoxLogFile.TabIndex = 18;
            this.textBoxLogFile.Click += new System.EventHandler(this.textBoxLogFile_Click);
            // 
            // saveFileDialogLogFile
            // 
            this.saveFileDialogLogFile.FileName = "ffmpeg-convert.log";
            this.saveFileDialogLogFile.Title = "选择日志文件";
            // 
            // textBoxLog
            // 
            this.textBoxLog.Location = new System.Drawing.Point(3, 3);
            this.textBoxLog.Multiline = true;
            this.textBoxLog.Name = "textBoxLog";
            this.textBoxLog.ReadOnly = true;
            this.textBoxLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBoxLog.Size = new System.Drawing.Size(577, 225);
            this.textBoxLog.TabIndex = 19;
            // 
            // buttonClear
            // 
            this.buttonClear.Location = new System.Drawing.Point(3, 468);
            this.buttonClear.Name = "buttonClear";
            this.buttonClear.Size = new System.Drawing.Size(105, 64);
            this.buttonClear.TabIndex = 20;
            this.buttonClear.Text = "清屏";
            this.buttonClear.UseVisualStyleBackColor = true;
            this.buttonClear.Click += new System.EventHandler(this.buttonClear_Click);
            // 
            // labelState
            // 
            this.labelState.AutoSize = true;
            this.labelState.ForeColor = System.Drawing.SystemColors.GrayText;
            this.labelState.Location = new System.Drawing.Point(3, 234);
            this.labelState.Name = "labelState";
            this.labelState.Size = new System.Drawing.Size(47, 12);
            this.labelState.TabIndex = 21;
            this.labelState.Text = "unknown";
            // 
            // checkBoxHiddenCmdline
            // 
            this.checkBoxHiddenCmdline.AutoSize = true;
            this.checkBoxHiddenCmdline.Location = new System.Drawing.Point(472, 260);
            this.checkBoxHiddenCmdline.Name = "checkBoxHiddenCmdline";
            this.checkBoxHiddenCmdline.Size = new System.Drawing.Size(108, 16);
            this.checkBoxHiddenCmdline.TabIndex = 22;
            this.checkBoxHiddenCmdline.Text = "隐藏命令提示符";
            this.checkBoxHiddenCmdline.UseVisualStyleBackColor = true;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 435);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(65, 12);
            this.label1.TabIndex = 23;
            this.label1.Text = "输出格式：";
            // 
            // comboBoxOutputFormat
            // 
            this.comboBoxOutputFormat.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxOutputFormat.FormattingEnabled = true;
            this.comboBoxOutputFormat.Items.AddRange(new object[] {
            "mp4",
            "h264"});
            this.comboBoxOutputFormat.Location = new System.Drawing.Point(86, 431);
            this.comboBoxOutputFormat.Name = "comboBoxOutputFormat";
            this.comboBoxOutputFormat.Size = new System.Drawing.Size(114, 20);
            this.comboBoxOutputFormat.TabIndex = 24;
            this.comboBoxOutputFormat.SelectedIndexChanged += new System.EventHandler(this.comboBoxOutputFormat_SelectedIndexChanged);
            // 
            // buttonInFile
            // 
            this.buttonInFile.Location = new System.Drawing.Point(494, 284);
            this.buttonInFile.Name = "buttonInFile";
            this.buttonInFile.Size = new System.Drawing.Size(40, 23);
            this.buttonInFile.TabIndex = 25;
            this.buttonInFile.Text = "文件";
            this.buttonInFile.UseVisualStyleBackColor = true;
            this.buttonInFile.Click += new System.EventHandler(this.buttonInFile_Click);
            // 
            // buttonInDir
            // 
            this.buttonInDir.Location = new System.Drawing.Point(540, 284);
            this.buttonInDir.Name = "buttonInDir";
            this.buttonInDir.Size = new System.Drawing.Size(40, 23);
            this.buttonInDir.TabIndex = 26;
            this.buttonInDir.Text = "目录";
            this.buttonInDir.UseVisualStyleBackColor = true;
            this.buttonInDir.Click += new System.EventHandler(this.buttonInDir_Click);
            // 
            // buttonOutDir
            // 
            this.buttonOutDir.Location = new System.Drawing.Point(540, 315);
            this.buttonOutDir.Name = "buttonOutDir";
            this.buttonOutDir.Size = new System.Drawing.Size(40, 23);
            this.buttonOutDir.TabIndex = 28;
            this.buttonOutDir.Text = "目录";
            this.buttonOutDir.UseVisualStyleBackColor = true;
            this.buttonOutDir.Click += new System.EventHandler(this.buttonOutDir_Click);
            // 
            // backgroundWorkerConvert
            // 
            this.backgroundWorkerConvert.WorkerSupportsCancellation = true;
            this.backgroundWorkerConvert.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorkerConvert_DoWork);
            // 
            // FFMpegConvert
            // 
            this.AcceptButton = this.buttonStart;
            this.AllowDrop = true;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 535);
            this.Controls.Add(this.buttonOutDir);
            this.Controls.Add(this.buttonInDir);
            this.Controls.Add(this.buttonInFile);
            this.Controls.Add(this.comboBoxOutputFormat);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.checkBoxHiddenCmdline);
            this.Controls.Add(this.labelState);
            this.Controls.Add(this.buttonClear);
            this.Controls.Add(this.textBoxLog);
            this.Controls.Add(this.textBoxLogFile);
            this.Controls.Add(this.labelLogFile);
            this.Controls.Add(this.checkBoxOverride);
            this.Controls.Add(this.textBoxParam);
            this.Controls.Add(this.labelVideoCodecParam);
            this.Controls.Add(this.buttonStop);
            this.Controls.Add(this.buttonStart);
            this.Controls.Add(this.textBoxOutPath);
            this.Controls.Add(this.textBoxInPath);
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
            this.KeyPreview = true;
            this.Name = "FFMpegConvert";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "视频格式转换器";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.FFMpegConvert_FormClosing);
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
        private System.Windows.Forms.TextBox textBoxInPath;
        private System.Windows.Forms.TextBox textBoxOutPath;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialogInDir;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialogOutDir;
        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.Button buttonStop;
        private System.Windows.Forms.Label labelVideoCodecParam;
        private System.Windows.Forms.TextBox textBoxParam;
        private System.Windows.Forms.CheckBox checkBoxOverride;
        private System.Windows.Forms.Label labelLogFile;
        private System.Windows.Forms.TextBox textBoxLogFile;
        private System.Windows.Forms.SaveFileDialog saveFileDialogLogFile;
        private System.Windows.Forms.TextBox textBoxLog;
        private System.Windows.Forms.Button buttonClear;
        private System.Windows.Forms.Label labelState;
        private System.Windows.Forms.CheckBox checkBoxHiddenCmdline;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox comboBoxOutputFormat;
        private System.Windows.Forms.Button buttonInFile;
        private System.Windows.Forms.Button buttonInDir;
        private System.Windows.Forms.Button buttonOutDir;
        private System.Windows.Forms.OpenFileDialog openFileDialogInFile;
        private System.ComponentModel.BackgroundWorker backgroundWorkerConvert;
    }
}

