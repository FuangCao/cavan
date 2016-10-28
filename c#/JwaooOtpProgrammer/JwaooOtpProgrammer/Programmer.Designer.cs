namespace JwaooOtpProgrammer {
    partial class Programmer {
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
            this.label1 = new System.Windows.Forms.Label();
            this.textBoxFirmware = new System.Windows.Forms.TextBox();
            this.buttonFirmware = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.buttonConnect = new System.Windows.Forms.Button();
            this.buttonBurn = new System.Windows.Forms.Button();
            this.textBoxBdAddressNext = new System.Windows.Forms.TextBox();
            this.openFileDialogFirmware = new System.Windows.Forms.OpenFileDialog();
            this.openFileDialogSmartSnippets = new System.Windows.Forms.OpenFileDialog();
            this.textBoxLog = new System.Windows.Forms.TextBox();
            this.buttonClearLog = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.textBoxBdAddressCurrent = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(16, 401);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(65, 12);
            this.label1.TabIndex = 0;
            this.label1.Text = "固件文件：";
            // 
            // textBoxFirmware
            // 
            this.textBoxFirmware.Location = new System.Drawing.Point(87, 400);
            this.textBoxFirmware.Name = "textBoxFirmware";
            this.textBoxFirmware.Size = new System.Drawing.Size(269, 21);
            this.textBoxFirmware.TabIndex = 1;
            // 
            // buttonFirmware
            // 
            this.buttonFirmware.Location = new System.Drawing.Point(362, 398);
            this.buttonFirmware.Name = "buttonFirmware";
            this.buttonFirmware.Size = new System.Drawing.Size(35, 23);
            this.buttonFirmware.TabIndex = 2;
            this.buttonFirmware.Text = "...";
            this.buttonFirmware.UseVisualStyleBackColor = true;
            this.buttonFirmware.Click += new System.EventHandler(this.buttonFirmware_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(4, 325);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(95, 12);
            this.label2.TabIndex = 3;
            this.label2.Text = "下一个MAC地址：";
            // 
            // buttonConnect
            // 
            this.buttonConnect.Location = new System.Drawing.Point(16, 440);
            this.buttonConnect.Name = "buttonConnect";
            this.buttonConnect.Size = new System.Drawing.Size(75, 23);
            this.buttonConnect.TabIndex = 5;
            this.buttonConnect.Text = "连接";
            this.buttonConnect.UseVisualStyleBackColor = true;
            this.buttonConnect.Click += new System.EventHandler(this.buttonConnect_Click);
            // 
            // buttonBurn
            // 
            this.buttonBurn.Enabled = false;
            this.buttonBurn.Location = new System.Drawing.Point(322, 438);
            this.buttonBurn.Name = "buttonBurn";
            this.buttonBurn.Size = new System.Drawing.Size(75, 23);
            this.buttonBurn.TabIndex = 6;
            this.buttonBurn.Text = "烧录";
            this.buttonBurn.UseVisualStyleBackColor = true;
            this.buttonBurn.Click += new System.EventHandler(this.buttonBurn_Click);
            // 
            // textBoxBdAddressNext
            // 
            this.textBoxBdAddressNext.Location = new System.Drawing.Point(105, 322);
            this.textBoxBdAddressNext.Name = "textBoxBdAddressNext";
            this.textBoxBdAddressNext.ReadOnly = true;
            this.textBoxBdAddressNext.Size = new System.Drawing.Size(292, 21);
            this.textBoxBdAddressNext.TabIndex = 4;
            // 
            // openFileDialogFirmware
            // 
            this.openFileDialogFirmware.FileName = "jwaoo-toy.hex";
            this.openFileDialogFirmware.Filter = "固件文件|*.hex";
            this.openFileDialogFirmware.Title = "选择固件文件";
            // 
            // openFileDialogSmartSnippets
            // 
            this.openFileDialogSmartSnippets.FileName = "SmartSnippets.exe";
            this.openFileDialogSmartSnippets.Filter = "SmartSnippets|SmartSnippets.exe";
            this.openFileDialogSmartSnippets.Title = "选择文件 SmartSnippets.exe";
            // 
            // textBoxLog
            // 
            this.textBoxLog.Location = new System.Drawing.Point(14, 12);
            this.textBoxLog.Multiline = true;
            this.textBoxLog.Name = "textBoxLog";
            this.textBoxLog.ReadOnly = true;
            this.textBoxLog.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBoxLog.Size = new System.Drawing.Size(383, 285);
            this.textBoxLog.TabIndex = 7;
            this.textBoxLog.WordWrap = false;
            // 
            // buttonClearLog
            // 
            this.buttonClearLog.Location = new System.Drawing.Point(152, 440);
            this.buttonClearLog.Name = "buttonClearLog";
            this.buttonClearLog.Size = new System.Drawing.Size(75, 23);
            this.buttonClearLog.TabIndex = 8;
            this.buttonClearLog.Text = "清空打印";
            this.buttonClearLog.UseVisualStyleBackColor = true;
            this.buttonClearLog.Click += new System.EventHandler(this.buttonClearLog_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(16, 363);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(83, 12);
            this.label3.TabIndex = 9;
            this.label3.Text = "当前MAC地址：";
            // 
            // textBoxBdAddressCurrent
            // 
            this.textBoxBdAddressCurrent.Location = new System.Drawing.Point(105, 360);
            this.textBoxBdAddressCurrent.Name = "textBoxBdAddressCurrent";
            this.textBoxBdAddressCurrent.ReadOnly = true;
            this.textBoxBdAddressCurrent.Size = new System.Drawing.Size(292, 21);
            this.textBoxBdAddressCurrent.TabIndex = 10;
            // 
            // Programmer
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(409, 475);
            this.Controls.Add(this.textBoxBdAddressCurrent);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.buttonClearLog);
            this.Controls.Add(this.textBoxLog);
            this.Controls.Add(this.buttonBurn);
            this.Controls.Add(this.buttonConnect);
            this.Controls.Add(this.textBoxBdAddressNext);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.buttonFirmware);
            this.Controls.Add(this.textBoxFirmware);
            this.Controls.Add(this.label1);
            this.Name = "Programmer";
            this.Text = "Form1";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.Programmer_FormClosed);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox textBoxFirmware;
        private System.Windows.Forms.Button buttonFirmware;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button buttonConnect;
        private System.Windows.Forms.Button buttonBurn;
        private System.Windows.Forms.TextBox textBoxBdAddressNext;
        private System.Windows.Forms.OpenFileDialog openFileDialogFirmware;
        private System.Windows.Forms.OpenFileDialog openFileDialogSmartSnippets;
        private System.Windows.Forms.TextBox textBoxLog;
        private System.Windows.Forms.Button buttonClearLog;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textBoxBdAddressCurrent;
    }
}

