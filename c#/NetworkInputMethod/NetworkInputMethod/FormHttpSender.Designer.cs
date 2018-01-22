namespace NetworkInputMethod
{
    partial class FormHttpSender
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormHttpSender));
            this.buttonStart = new System.Windows.Forms.Button();
            this.buttonStop = new System.Windows.Forms.Button();
            this.dateTimePickerStart = new System.Windows.Forms.DateTimePicker();
            this.comboBoxDelay = new System.Windows.Forms.ComboBox();
            this.textBoxPath = new System.Windows.Forms.TextBox();
            this.buttonOpen = new System.Windows.Forms.Button();
            this.labelStatus = new System.Windows.Forms.Label();
            this.textBoxLog = new System.Windows.Forms.TextBox();
            this.openFileDialogReq = new System.Windows.Forms.OpenFileDialog();
            this.buttonClear = new System.Windows.Forms.Button();
            this.timerWait = new System.Windows.Forms.Timer(this.components);
            this.buttonNow = new System.Windows.Forms.Button();
            this.buttonEdit = new System.Windows.Forms.Button();
            this.checkBoxDebug = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // buttonStart
            // 
            this.buttonStart.Location = new System.Drawing.Point(404, 425);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(75, 23);
            this.buttonStart.TabIndex = 0;
            this.buttonStart.Text = "启动";
            this.buttonStart.UseVisualStyleBackColor = true;
            this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
            // 
            // buttonStop
            // 
            this.buttonStop.Location = new System.Drawing.Point(323, 425);
            this.buttonStop.Name = "buttonStop";
            this.buttonStop.Size = new System.Drawing.Size(75, 23);
            this.buttonStop.TabIndex = 1;
            this.buttonStop.Text = "停止";
            this.buttonStop.UseVisualStyleBackColor = true;
            this.buttonStop.Click += new System.EventHandler(this.buttonStop_Click);
            // 
            // dateTimePickerStart
            // 
            this.dateTimePickerStart.Format = System.Windows.Forms.DateTimePickerFormat.Custom;
            this.dateTimePickerStart.Location = new System.Drawing.Point(12, 345);
            this.dateTimePickerStart.Name = "dateTimePickerStart";
            this.dateTimePickerStart.ShowUpDown = true;
            this.dateTimePickerStart.Size = new System.Drawing.Size(285, 21);
            this.dateTimePickerStart.TabIndex = 2;
            // 
            // comboBoxDelay
            // 
            this.comboBoxDelay.FormattingEnabled = true;
            this.comboBoxDelay.Items.AddRange(new object[] {
            "提前3000毫秒",
            "提前2900毫秒",
            "提前2800毫秒",
            "提前2700毫秒",
            "提前2600毫秒",
            "提前2500毫秒",
            "提前2400毫秒",
            "提前2300毫秒",
            "提前2200毫秒",
            "提前2100毫秒",
            "提前2000毫秒",
            "提前1900毫秒",
            "提前1800毫秒",
            "提前1700毫秒",
            "提前1600毫秒",
            "提前1500毫秒",
            "提前1400毫秒",
            "提前1300毫秒",
            "提前1200毫秒",
            "提前1100毫秒",
            "提前1000毫秒",
            "提前900毫秒",
            "提前800毫秒",
            "提前700毫秒",
            "提前600毫秒",
            "提前500毫秒",
            "提前400毫秒",
            "提前300毫秒",
            "提前200毫秒",
            "提前100毫秒",
            "准时执行",
            "延迟100毫秒",
            "延迟200毫秒",
            "延迟300毫秒",
            "延迟400毫秒",
            "延迟500毫秒",
            "延迟600毫秒",
            "延迟700毫秒",
            "延迟800毫秒",
            "延迟900毫秒",
            "延迟1000毫秒",
            "延迟1100毫秒",
            "延迟1200毫秒",
            "延迟1300毫秒",
            "延迟1400毫秒",
            "延迟1500毫秒",
            "延迟1600毫秒",
            "延迟1700毫秒",
            "延迟1800毫秒",
            "延迟1900毫秒",
            "延迟2000毫秒",
            "延迟2100毫秒",
            "延迟2200毫秒",
            "延迟2300毫秒",
            "延迟2400毫秒",
            "延迟2500毫秒",
            "延迟2600毫秒",
            "延迟2700毫秒",
            "延迟2800毫秒",
            "延迟2900毫秒",
            "延迟3000毫秒"});
            this.comboBoxDelay.Location = new System.Drawing.Point(303, 345);
            this.comboBoxDelay.Name = "comboBoxDelay";
            this.comboBoxDelay.Size = new System.Drawing.Size(176, 20);
            this.comboBoxDelay.TabIndex = 3;
            // 
            // textBoxPath
            // 
            this.textBoxPath.Location = new System.Drawing.Point(12, 371);
            this.textBoxPath.Name = "textBoxPath";
            this.textBoxPath.ReadOnly = true;
            this.textBoxPath.Size = new System.Drawing.Size(424, 21);
            this.textBoxPath.TabIndex = 4;
            // 
            // buttonOpen
            // 
            this.buttonOpen.Location = new System.Drawing.Point(442, 369);
            this.buttonOpen.Name = "buttonOpen";
            this.buttonOpen.Size = new System.Drawing.Size(37, 23);
            this.buttonOpen.TabIndex = 5;
            this.buttonOpen.Text = "...";
            this.buttonOpen.UseVisualStyleBackColor = true;
            this.buttonOpen.Click += new System.EventHandler(this.buttonOpen_Click);
            // 
            // labelStatus
            // 
            this.labelStatus.Location = new System.Drawing.Point(10, 399);
            this.labelStatus.Name = "labelStatus";
            this.labelStatus.Size = new System.Drawing.Size(467, 12);
            this.labelStatus.TabIndex = 6;
            this.labelStatus.Text = "请点击启动";
            // 
            // textBoxLog
            // 
            this.textBoxLog.Location = new System.Drawing.Point(14, 12);
            this.textBoxLog.Multiline = true;
            this.textBoxLog.Name = "textBoxLog";
            this.textBoxLog.ReadOnly = true;
            this.textBoxLog.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBoxLog.Size = new System.Drawing.Size(467, 327);
            this.textBoxLog.TabIndex = 7;
            // 
            // openFileDialogReq
            // 
            this.openFileDialogReq.FileName = "C:\\weixin\\game.txt";
            this.openFileDialogReq.Filter = "文本文件|*.txt";
            this.openFileDialogReq.InitialDirectory = "C:\\weixin";
            this.openFileDialogReq.ShowReadOnly = true;
            // 
            // buttonClear
            // 
            this.buttonClear.Location = new System.Drawing.Point(12, 425);
            this.buttonClear.Name = "buttonClear";
            this.buttonClear.Size = new System.Drawing.Size(75, 23);
            this.buttonClear.TabIndex = 8;
            this.buttonClear.Text = "清空";
            this.buttonClear.UseVisualStyleBackColor = true;
            this.buttonClear.Click += new System.EventHandler(this.buttonClear_Click);
            // 
            // timerWait
            // 
            this.timerWait.Interval = 1000;
            this.timerWait.Tick += new System.EventHandler(this.timerWait_Tick);
            // 
            // buttonNow
            // 
            this.buttonNow.Location = new System.Drawing.Point(93, 425);
            this.buttonNow.Name = "buttonNow";
            this.buttonNow.Size = new System.Drawing.Size(75, 23);
            this.buttonNow.TabIndex = 9;
            this.buttonNow.Text = "现在";
            this.buttonNow.UseVisualStyleBackColor = true;
            this.buttonNow.Click += new System.EventHandler(this.buttonNow_Click);
            // 
            // buttonEdit
            // 
            this.buttonEdit.Location = new System.Drawing.Point(174, 425);
            this.buttonEdit.Name = "buttonEdit";
            this.buttonEdit.Size = new System.Drawing.Size(75, 23);
            this.buttonEdit.TabIndex = 10;
            this.buttonEdit.Text = "编辑";
            this.buttonEdit.UseVisualStyleBackColor = true;
            this.buttonEdit.Click += new System.EventHandler(this.buttonEdit_Click);
            // 
            // checkBoxDebug
            // 
            this.checkBoxDebug.AutoSize = true;
            this.checkBoxDebug.Location = new System.Drawing.Point(431, 398);
            this.checkBoxDebug.Name = "checkBoxDebug";
            this.checkBoxDebug.Size = new System.Drawing.Size(48, 16);
            this.checkBoxDebug.TabIndex = 11;
            this.checkBoxDebug.Text = "调试";
            this.checkBoxDebug.UseVisualStyleBackColor = true;
            // 
            // FormHttpSender
            // 
            this.AcceptButton = this.buttonStart;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(491, 460);
            this.Controls.Add(this.checkBoxDebug);
            this.Controls.Add(this.buttonEdit);
            this.Controls.Add(this.buttonNow);
            this.Controls.Add(this.buttonClear);
            this.Controls.Add(this.textBoxLog);
            this.Controls.Add(this.labelStatus);
            this.Controls.Add(this.buttonOpen);
            this.Controls.Add(this.textBoxPath);
            this.Controls.Add(this.comboBoxDelay);
            this.Controls.Add(this.dateTimePickerStart);
            this.Controls.Add(this.buttonStop);
            this.Controls.Add(this.buttonStart);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "FormHttpSender";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "HTTP请求发送器";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.Button buttonStop;
        private System.Windows.Forms.DateTimePicker dateTimePickerStart;
        private System.Windows.Forms.ComboBox comboBoxDelay;
        private System.Windows.Forms.TextBox textBoxPath;
        private System.Windows.Forms.Button buttonOpen;
        private System.Windows.Forms.Label labelStatus;
        private System.Windows.Forms.TextBox textBoxLog;
        private System.Windows.Forms.OpenFileDialog openFileDialogReq;
        private System.Windows.Forms.Button buttonClear;
        private System.Windows.Forms.Timer timerWait;
        private System.Windows.Forms.Button buttonNow;
        private System.Windows.Forms.Button buttonEdit;
        private System.Windows.Forms.CheckBox checkBoxDebug;
    }
}