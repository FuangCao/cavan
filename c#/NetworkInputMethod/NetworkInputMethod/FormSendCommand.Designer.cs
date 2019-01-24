namespace NetworkInputMethod
{
    partial class FormSendCommand
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.textBoxCommand = new System.Windows.Forms.TextBox();
            this.buttonSend = new System.Windows.Forms.Button();
            this.buttonView = new System.Windows.Forms.Button();
            this.textBoxResponse = new System.Windows.Forms.TextBox();
            this.buttonClear = new System.Windows.Forms.Button();
            this.comboBoxDevices = new System.Windows.Forms.ComboBox();
            this.buttonTest = new System.Windows.Forms.Button();
            this.comboBoxRepeat = new System.Windows.Forms.ComboBox();
            this.buttonStart = new System.Windows.Forms.Button();
            this.buttonStop = new System.Windows.Forms.Button();
            this.buttonVip = new System.Windows.Forms.Button();
            this.buttonXfzd = new System.Windows.Forms.Button();
            this.timerRepeater = new System.Windows.Forms.Timer(this.components);
            this.labelStatus = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // textBoxCommand
            // 
            this.textBoxCommand.Location = new System.Drawing.Point(12, 488);
            this.textBoxCommand.Name = "textBoxCommand";
            this.textBoxCommand.Size = new System.Drawing.Size(760, 21);
            this.textBoxCommand.TabIndex = 0;
            // 
            // buttonSend
            // 
            this.buttonSend.Location = new System.Drawing.Point(697, 526);
            this.buttonSend.Name = "buttonSend";
            this.buttonSend.Size = new System.Drawing.Size(75, 23);
            this.buttonSend.TabIndex = 1;
            this.buttonSend.Text = "发送";
            this.buttonSend.UseVisualStyleBackColor = true;
            this.buttonSend.Click += new System.EventHandler(this.buttonSend_Click);
            // 
            // buttonView
            // 
            this.buttonView.Location = new System.Drawing.Point(12, 526);
            this.buttonView.Name = "buttonView";
            this.buttonView.Size = new System.Drawing.Size(75, 23);
            this.buttonView.TabIndex = 3;
            this.buttonView.Text = "浏览";
            this.buttonView.UseVisualStyleBackColor = true;
            this.buttonView.Click += new System.EventHandler(this.buttonView_Click);
            // 
            // textBoxResponse
            // 
            this.textBoxResponse.Location = new System.Drawing.Point(12, 38);
            this.textBoxResponse.Multiline = true;
            this.textBoxResponse.Name = "textBoxResponse";
            this.textBoxResponse.ReadOnly = true;
            this.textBoxResponse.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBoxResponse.Size = new System.Drawing.Size(760, 444);
            this.textBoxResponse.TabIndex = 4;
            // 
            // buttonClear
            // 
            this.buttonClear.Location = new System.Drawing.Point(616, 526);
            this.buttonClear.Name = "buttonClear";
            this.buttonClear.Size = new System.Drawing.Size(75, 23);
            this.buttonClear.TabIndex = 5;
            this.buttonClear.Text = "清除";
            this.buttonClear.UseVisualStyleBackColor = true;
            this.buttonClear.Click += new System.EventHandler(this.buttonClear_Click);
            // 
            // comboBoxDevices
            // 
            this.comboBoxDevices.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxDevices.FormattingEnabled = true;
            this.comboBoxDevices.Location = new System.Drawing.Point(12, 12);
            this.comboBoxDevices.Name = "comboBoxDevices";
            this.comboBoxDevices.Size = new System.Drawing.Size(217, 20);
            this.comboBoxDevices.TabIndex = 6;
            this.comboBoxDevices.DropDown += new System.EventHandler(this.comboBoxDevices_DropDown);
            // 
            // buttonTest
            // 
            this.buttonTest.Location = new System.Drawing.Point(93, 526);
            this.buttonTest.Name = "buttonTest";
            this.buttonTest.Size = new System.Drawing.Size(75, 23);
            this.buttonTest.TabIndex = 7;
            this.buttonTest.Text = "测试";
            this.buttonTest.UseVisualStyleBackColor = true;
            this.buttonTest.Click += new System.EventHandler(this.buttonTest_Click);
            // 
            // comboBoxRepeat
            // 
            this.comboBoxRepeat.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxRepeat.FormattingEnabled = true;
            this.comboBoxRepeat.Items.AddRange(new object[] {
            "不重复发送",
            "重复间隔50毫秒",
            "重复间隔100毫秒",
            "重复间隔150毫秒",
            "重复间隔200毫秒",
            "重复间隔250毫秒",
            "重复间隔300毫秒",
            "重复间隔350毫秒",
            "重复间隔400毫秒",
            "重复间隔450毫秒",
            "重复间隔500毫秒",
            "重复间隔550毫秒",
            "重复间隔600毫秒",
            "重复间隔650毫秒",
            "重复间隔700毫秒",
            "重复间隔750毫秒",
            "重复间隔800毫秒",
            "重复间隔850毫秒",
            "重复间隔900毫秒",
            "重复间隔950毫秒",
            "重复间隔1000毫秒",
            "重复间隔1500毫秒",
            "重复间隔2000毫秒",
            "重复间隔2500毫秒",
            "重复间隔3000毫秒",
            "重复间隔3500毫秒",
            "重复间隔4000毫秒",
            "重复间隔4500毫秒",
            "重复间隔5000毫秒"});
            this.comboBoxRepeat.Location = new System.Drawing.Point(235, 12);
            this.comboBoxRepeat.Name = "comboBoxRepeat";
            this.comboBoxRepeat.Size = new System.Drawing.Size(217, 20);
            this.comboBoxRepeat.TabIndex = 8;
            this.comboBoxRepeat.SelectedIndexChanged += new System.EventHandler(this.comboBoxRepeat_SelectedIndexChanged);
            // 
            // buttonStart
            // 
            this.buttonStart.Location = new System.Drawing.Point(336, 526);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(75, 23);
            this.buttonStart.TabIndex = 9;
            this.buttonStart.Text = "开始";
            this.buttonStart.UseVisualStyleBackColor = true;
            this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
            // 
            // buttonStop
            // 
            this.buttonStop.Location = new System.Drawing.Point(535, 526);
            this.buttonStop.Name = "buttonStop";
            this.buttonStop.Size = new System.Drawing.Size(75, 23);
            this.buttonStop.TabIndex = 10;
            this.buttonStop.Text = "停止";
            this.buttonStop.UseVisualStyleBackColor = true;
            this.buttonStop.Click += new System.EventHandler(this.buttonStop_Click);
            // 
            // buttonVip
            // 
            this.buttonVip.Location = new System.Drawing.Point(174, 526);
            this.buttonVip.Name = "buttonVip";
            this.buttonVip.Size = new System.Drawing.Size(75, 23);
            this.buttonVip.TabIndex = 11;
            this.buttonVip.Text = "VIP";
            this.buttonVip.UseVisualStyleBackColor = true;
            this.buttonVip.Click += new System.EventHandler(this.buttonVip_Click);
            // 
            // buttonXfzd
            // 
            this.buttonXfzd.Location = new System.Drawing.Point(255, 526);
            this.buttonXfzd.Name = "buttonXfzd";
            this.buttonXfzd.Size = new System.Drawing.Size(75, 23);
            this.buttonXfzd.TabIndex = 12;
            this.buttonXfzd.Text = "幸福账单";
            this.buttonXfzd.UseVisualStyleBackColor = true;
            this.buttonXfzd.Click += new System.EventHandler(this.buttonXfzd_Click);
            // 
            // timerRepeater
            // 
            this.timerRepeater.Interval = 1000;
            this.timerRepeater.Tick += new System.EventHandler(this.timerRepeater_Tick);
            // 
            // labelStatus
            // 
            this.labelStatus.Location = new System.Drawing.Point(458, 15);
            this.labelStatus.Name = "labelStatus";
            this.labelStatus.Size = new System.Drawing.Size(314, 15);
            this.labelStatus.TabIndex = 13;
            this.labelStatus.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // FormSendCommand
            // 
            this.AcceptButton = this.buttonSend;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(784, 561);
            this.Controls.Add(this.labelStatus);
            this.Controls.Add(this.buttonXfzd);
            this.Controls.Add(this.buttonVip);
            this.Controls.Add(this.buttonStop);
            this.Controls.Add(this.buttonStart);
            this.Controls.Add(this.comboBoxRepeat);
            this.Controls.Add(this.buttonTest);
            this.Controls.Add(this.comboBoxDevices);
            this.Controls.Add(this.buttonClear);
            this.Controls.Add(this.textBoxResponse);
            this.Controls.Add(this.buttonView);
            this.Controls.Add(this.buttonSend);
            this.Controls.Add(this.textBoxCommand);
            this.Name = "FormSendCommand";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "发送命令";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.FormSendCommand_FormClosing);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox textBoxCommand;
        private System.Windows.Forms.Button buttonSend;
        private System.Windows.Forms.Button buttonView;
        private System.Windows.Forms.TextBox textBoxResponse;
        private System.Windows.Forms.Button buttonClear;
        private System.Windows.Forms.ComboBox comboBoxDevices;
        private System.Windows.Forms.Button buttonTest;
        private System.Windows.Forms.ComboBox comboBoxRepeat;
        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.Button buttonStop;
        private System.Windows.Forms.Button buttonVip;
        private System.Windows.Forms.Button buttonXfzd;
        private System.Windows.Forms.Timer timerRepeater;
        private System.Windows.Forms.Label labelStatus;
    }
}