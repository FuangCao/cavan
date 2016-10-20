namespace JwaooOtpProgrammer {
    partial class Form1 {
        /// <summary>
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing) {
            if (disposing && (components != null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows 窗体设计器生成的代码

        /// <summary>
        /// 设计器支持所需的方法 - 不要修改
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent() {
            this.folderBrowserDialogFirmware = new System.Windows.Forms.FolderBrowserDialog();
            this.label1 = new System.Windows.Forms.Label();
            this.textBoxFirmware = new System.Windows.Forms.TextBox();
            this.buttonFirmware = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.buttonConnect = new System.Windows.Forms.Button();
            this.buttonBurn = new System.Windows.Forms.Button();
            this.textBoxLog = new System.Windows.Forms.TextBox();
            this.buttonLogClean = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // folderBrowserDialogFirmware
            // 
            this.folderBrowserDialogFirmware.ShowNewFolderButton = false;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(17, 476);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(65, 12);
            this.label1.TabIndex = 0;
            this.label1.Text = "固件文件：";
            // 
            // textBoxFirmware
            // 
            this.textBoxFirmware.Location = new System.Drawing.Point(88, 473);
            this.textBoxFirmware.Name = "textBoxFirmware";
            this.textBoxFirmware.Size = new System.Drawing.Size(738, 21);
            this.textBoxFirmware.TabIndex = 1;
            // 
            // buttonFirmware
            // 
            this.buttonFirmware.Location = new System.Drawing.Point(832, 471);
            this.buttonFirmware.Name = "buttonFirmware";
            this.buttonFirmware.Size = new System.Drawing.Size(47, 23);
            this.buttonFirmware.TabIndex = 2;
            this.buttonFirmware.Text = "浏览";
            this.buttonFirmware.UseVisualStyleBackColor = true;
            this.buttonFirmware.Click += new System.EventHandler(this.buttonFirmware_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(17, 438);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(65, 12);
            this.label2.TabIndex = 3;
            this.label2.Text = "硬件地址：";
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(91, 435);
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(788, 21);
            this.textBox1.TabIndex = 4;
            // 
            // buttonConnect
            // 
            this.buttonConnect.Location = new System.Drawing.Point(12, 524);
            this.buttonConnect.Name = "buttonConnect";
            this.buttonConnect.Size = new System.Drawing.Size(75, 23);
            this.buttonConnect.TabIndex = 5;
            this.buttonConnect.Text = "连接";
            this.buttonConnect.UseVisualStyleBackColor = true;
            this.buttonConnect.Click += new System.EventHandler(this.buttonConnect_Click);
            // 
            // buttonBurn
            // 
            this.buttonBurn.Location = new System.Drawing.Point(802, 524);
            this.buttonBurn.Name = "buttonBurn";
            this.buttonBurn.Size = new System.Drawing.Size(75, 23);
            this.buttonBurn.TabIndex = 6;
            this.buttonBurn.Text = "烧录";
            this.buttonBurn.UseVisualStyleBackColor = true;
            this.buttonBurn.Click += new System.EventHandler(this.buttonBurn_Click);
            // 
            // textBoxLog
            // 
            this.textBoxLog.Location = new System.Drawing.Point(12, 12);
            this.textBoxLog.Multiline = true;
            this.textBoxLog.Name = "textBoxLog";
            this.textBoxLog.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBoxLog.Size = new System.Drawing.Size(865, 408);
            this.textBoxLog.TabIndex = 7;
            // 
            // buttonLogClean
            // 
            this.buttonLogClean.Location = new System.Drawing.Point(384, 524);
            this.buttonLogClean.Name = "buttonLogClean";
            this.buttonLogClean.Size = new System.Drawing.Size(75, 23);
            this.buttonLogClean.TabIndex = 8;
            this.buttonLogClean.Text = "清空消息";
            this.buttonLogClean.UseVisualStyleBackColor = true;
            this.buttonLogClean.Click += new System.EventHandler(this.buttonLogClean_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(889, 559);
            this.Controls.Add(this.buttonLogClean);
            this.Controls.Add(this.textBoxLog);
            this.Controls.Add(this.buttonBurn);
            this.Controls.Add(this.buttonConnect);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.buttonFirmware);
            this.Controls.Add(this.textBoxFirmware);
            this.Controls.Add(this.label1);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialogFirmware;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox textBoxFirmware;
        private System.Windows.Forms.Button buttonFirmware;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.Button buttonConnect;
        private System.Windows.Forms.Button buttonBurn;
        private System.Windows.Forms.TextBox textBoxLog;
        private System.Windows.Forms.Button buttonLogClean;
    }
}

