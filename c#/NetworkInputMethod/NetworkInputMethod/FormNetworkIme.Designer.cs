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
            this.checkedListBoxClients = new System.Windows.Forms.CheckedListBox();
            this.textBoxContent = new System.Windows.Forms.TextBox();
            this.buttonSend = new System.Windows.Forms.Button();
            this.buttonClear = new System.Windows.Forms.Button();
            this.groupBoxMode = new System.Windows.Forms.GroupBox();
            this.radioButtonReplace = new System.Windows.Forms.RadioButton();
            this.radioButtonInsert = new System.Windows.Forms.RadioButton();
            this.radioButtonSend = new System.Windows.Forms.RadioButton();
            this.checkBoxClear = new System.Windows.Forms.CheckBox();
            this.buttonBack = new System.Windows.Forms.Button();
            this.buttonUp = new System.Windows.Forms.Button();
            this.buttonDown = new System.Windows.Forms.Button();
            this.buttonLeft = new System.Windows.Forms.Button();
            this.buttonRight = new System.Windows.Forms.Button();
            this.buttonEnter = new System.Windows.Forms.Button();
            this.labelStatus = new System.Windows.Forms.Label();
            this.checkBoxRepeat = new System.Windows.Forms.CheckBox();
            this.checkBoxEnterSend = new System.Windows.Forms.CheckBox();
            this.backgroundWorkerRepeater = new System.ComponentModel.BackgroundWorker();
            this.checkBoxShareClipboard = new System.Windows.Forms.CheckBox();
            this.checkBoxSelectAll = new System.Windows.Forms.CheckBox();
            this.buttonDelete = new System.Windows.Forms.Button();
            this.buttonMm = new System.Windows.Forms.Button();
            this.buttonSelectAll = new System.Windows.Forms.Button();
            this.buttonCopy = new System.Windows.Forms.Button();
            this.buttonPaste = new System.Windows.Forms.Button();
            this.buttonComplete = new System.Windows.Forms.Button();
            this.groupBoxMode.SuspendLayout();
            this.SuspendLayout();
            // 
            // labelPort
            // 
            this.labelPort.AutoSize = true;
            this.labelPort.Location = new System.Drawing.Point(12, 16);
            this.labelPort.Name = "labelPort";
            this.labelPort.Size = new System.Drawing.Size(53, 12);
            this.labelPort.TabIndex = 0;
            this.labelPort.Text = "端口号：";
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(71, 12);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(304, 21);
            this.textBoxPort.TabIndex = 1;
            this.textBoxPort.Text = "8865";
            // 
            // buttonStart
            // 
            this.buttonStart.Location = new System.Drawing.Point(381, 10);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(75, 23);
            this.buttonStart.TabIndex = 2;
            this.buttonStart.Text = "启动";
            this.buttonStart.UseVisualStyleBackColor = true;
            this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
            // 
            // checkedListBoxClients
            // 
            this.checkedListBoxClients.CheckOnClick = true;
            this.checkedListBoxClients.FormattingEnabled = true;
            this.checkedListBoxClients.Location = new System.Drawing.Point(14, 39);
            this.checkedListBoxClients.Name = "checkedListBoxClients";
            this.checkedListBoxClients.Size = new System.Drawing.Size(442, 132);
            this.checkedListBoxClients.TabIndex = 4;
            this.checkedListBoxClients.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.checkedListBoxClients_ItemCheck);
            // 
            // textBoxContent
            // 
            this.textBoxContent.Location = new System.Drawing.Point(14, 199);
            this.textBoxContent.Multiline = true;
            this.textBoxContent.Name = "textBoxContent";
            this.textBoxContent.Size = new System.Drawing.Size(442, 153);
            this.textBoxContent.TabIndex = 5;
            // 
            // buttonSend
            // 
            this.buttonSend.Location = new System.Drawing.Point(381, 478);
            this.buttonSend.Name = "buttonSend";
            this.buttonSend.Size = new System.Drawing.Size(75, 23);
            this.buttonSend.TabIndex = 6;
            this.buttonSend.Text = "发送";
            this.buttonSend.UseVisualStyleBackColor = true;
            this.buttonSend.Click += new System.EventHandler(this.buttonSend_Click);
            // 
            // buttonClear
            // 
            this.buttonClear.Location = new System.Drawing.Point(300, 478);
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
            this.groupBoxMode.Location = new System.Drawing.Point(381, 380);
            this.groupBoxMode.Name = "groupBoxMode";
            this.groupBoxMode.Size = new System.Drawing.Size(75, 94);
            this.groupBoxMode.TabIndex = 8;
            this.groupBoxMode.TabStop = false;
            // 
            // radioButtonReplace
            // 
            this.radioButtonReplace.AutoSize = true;
            this.radioButtonReplace.Location = new System.Drawing.Point(13, 42);
            this.radioButtonReplace.Name = "radioButtonReplace";
            this.radioButtonReplace.Size = new System.Drawing.Size(47, 16);
            this.radioButtonReplace.TabIndex = 1;
            this.radioButtonReplace.Text = "替换";
            this.radioButtonReplace.UseVisualStyleBackColor = true;
            this.radioButtonReplace.CheckedChanged += new System.EventHandler(this.radioButtonReplace_CheckedChanged);
            // 
            // radioButtonInsert
            // 
            this.radioButtonInsert.AutoSize = true;
            this.radioButtonInsert.Location = new System.Drawing.Point(13, 64);
            this.radioButtonInsert.Name = "radioButtonInsert";
            this.radioButtonInsert.Size = new System.Drawing.Size(47, 16);
            this.radioButtonInsert.TabIndex = 0;
            this.radioButtonInsert.Text = "插入";
            this.radioButtonInsert.UseVisualStyleBackColor = true;
            this.radioButtonInsert.CheckedChanged += new System.EventHandler(this.radioButtonInsert_CheckedChanged);
            // 
            // radioButtonSend
            // 
            this.radioButtonSend.AutoSize = true;
            this.radioButtonSend.Checked = true;
            this.radioButtonSend.Location = new System.Drawing.Point(13, 20);
            this.radioButtonSend.Name = "radioButtonSend";
            this.radioButtonSend.Size = new System.Drawing.Size(47, 16);
            this.radioButtonSend.TabIndex = 2;
            this.radioButtonSend.TabStop = true;
            this.radioButtonSend.Text = "发送";
            this.radioButtonSend.UseVisualStyleBackColor = true;
            this.radioButtonSend.CheckedChanged += new System.EventHandler(this.radioButtonSend_CheckedChanged);
            // 
            // checkBoxClear
            // 
            this.checkBoxClear.AutoSize = true;
            this.checkBoxClear.Location = new System.Drawing.Point(291, 358);
            this.checkBoxClear.Name = "checkBoxClear";
            this.checkBoxClear.Size = new System.Drawing.Size(84, 16);
            this.checkBoxClear.TabIndex = 9;
            this.checkBoxClear.Text = "发送后清除";
            this.checkBoxClear.UseVisualStyleBackColor = true;
            // 
            // buttonBack
            // 
            this.buttonBack.Location = new System.Drawing.Point(95, 478);
            this.buttonBack.Name = "buttonBack";
            this.buttonBack.Size = new System.Drawing.Size(75, 23);
            this.buttonBack.TabIndex = 10;
            this.buttonBack.Text = "返回";
            this.buttonBack.UseVisualStyleBackColor = true;
            this.buttonBack.Click += new System.EventHandler(this.buttonBack_Click);
            // 
            // buttonUp
            // 
            this.buttonUp.Location = new System.Drawing.Point(93, 393);
            this.buttonUp.Name = "buttonUp";
            this.buttonUp.Size = new System.Drawing.Size(75, 23);
            this.buttonUp.TabIndex = 13;
            this.buttonUp.Text = "上移";
            this.buttonUp.UseVisualStyleBackColor = true;
            this.buttonUp.Click += new System.EventHandler(this.buttonUp_Click);
            // 
            // buttonDown
            // 
            this.buttonDown.Location = new System.Drawing.Point(93, 449);
            this.buttonDown.Name = "buttonDown";
            this.buttonDown.Size = new System.Drawing.Size(75, 23);
            this.buttonDown.TabIndex = 14;
            this.buttonDown.Text = "下移";
            this.buttonDown.UseVisualStyleBackColor = true;
            this.buttonDown.Click += new System.EventHandler(this.buttonDown_Click);
            // 
            // buttonLeft
            // 
            this.buttonLeft.Location = new System.Drawing.Point(12, 421);
            this.buttonLeft.Name = "buttonLeft";
            this.buttonLeft.Size = new System.Drawing.Size(75, 23);
            this.buttonLeft.TabIndex = 15;
            this.buttonLeft.Text = "左移";
            this.buttonLeft.UseVisualStyleBackColor = true;
            this.buttonLeft.Click += new System.EventHandler(this.buttonLeft_Click);
            // 
            // buttonRight
            // 
            this.buttonRight.Location = new System.Drawing.Point(174, 421);
            this.buttonRight.Name = "buttonRight";
            this.buttonRight.Size = new System.Drawing.Size(75, 23);
            this.buttonRight.TabIndex = 16;
            this.buttonRight.Text = "右移";
            this.buttonRight.UseVisualStyleBackColor = true;
            this.buttonRight.Click += new System.EventHandler(this.buttonRight_Click);
            // 
            // buttonEnter
            // 
            this.buttonEnter.Location = new System.Drawing.Point(93, 421);
            this.buttonEnter.Name = "buttonEnter";
            this.buttonEnter.Size = new System.Drawing.Size(75, 23);
            this.buttonEnter.TabIndex = 19;
            this.buttonEnter.Text = "确定";
            this.buttonEnter.UseVisualStyleBackColor = true;
            this.buttonEnter.Click += new System.EventHandler(this.buttonEnter_Click);
            // 
            // labelStatus
            // 
            this.labelStatus.AutoSize = true;
            this.labelStatus.Location = new System.Drawing.Point(12, 360);
            this.labelStatus.Name = "labelStatus";
            this.labelStatus.Size = new System.Drawing.Size(77, 12);
            this.labelStatus.TabIndex = 20;
            this.labelStatus.Text = "服务器未启动";
            // 
            // checkBoxRepeat
            // 
            this.checkBoxRepeat.AutoSize = true;
            this.checkBoxRepeat.Location = new System.Drawing.Point(384, 177);
            this.checkBoxRepeat.Name = "checkBoxRepeat";
            this.checkBoxRepeat.Size = new System.Drawing.Size(72, 16);
            this.checkBoxRepeat.TabIndex = 21;
            this.checkBoxRepeat.Text = "重复发送";
            this.checkBoxRepeat.UseVisualStyleBackColor = true;
            this.checkBoxRepeat.CheckedChanged += new System.EventHandler(this.checkBoxRepeat_CheckedChanged);
            // 
            // checkBoxEnterSend
            // 
            this.checkBoxEnterSend.AutoSize = true;
            this.checkBoxEnterSend.Checked = true;
            this.checkBoxEnterSend.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxEnterSend.Location = new System.Drawing.Point(384, 358);
            this.checkBoxEnterSend.Name = "checkBoxEnterSend";
            this.checkBoxEnterSend.Size = new System.Drawing.Size(72, 16);
            this.checkBoxEnterSend.TabIndex = 22;
            this.checkBoxEnterSend.Text = "回车发送";
            this.checkBoxEnterSend.UseVisualStyleBackColor = true;
            // 
            // backgroundWorkerRepeater
            // 
            this.backgroundWorkerRepeater.WorkerSupportsCancellation = true;
            this.backgroundWorkerRepeater.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorkerRepeater_DoWork);
            this.backgroundWorkerRepeater.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.backgroundWorkerRepeater_RunWorkerCompleted);
            // 
            // checkBoxShareClipboard
            // 
            this.checkBoxShareClipboard.AutoSize = true;
            this.checkBoxShareClipboard.Checked = true;
            this.checkBoxShareClipboard.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxShareClipboard.Location = new System.Drawing.Point(294, 177);
            this.checkBoxShareClipboard.Name = "checkBoxShareClipboard";
            this.checkBoxShareClipboard.Size = new System.Drawing.Size(84, 16);
            this.checkBoxShareClipboard.TabIndex = 23;
            this.checkBoxShareClipboard.Text = "剪切板分享";
            this.checkBoxShareClipboard.UseVisualStyleBackColor = true;
            // 
            // checkBoxSelectAll
            // 
            this.checkBoxSelectAll.AutoSize = true;
            this.checkBoxSelectAll.Location = new System.Drawing.Point(14, 177);
            this.checkBoxSelectAll.Name = "checkBoxSelectAll";
            this.checkBoxSelectAll.Size = new System.Drawing.Size(48, 16);
            this.checkBoxSelectAll.TabIndex = 24;
            this.checkBoxSelectAll.Text = "全选";
            this.checkBoxSelectAll.UseVisualStyleBackColor = true;
            this.checkBoxSelectAll.CheckedChanged += new System.EventHandler(this.checkBoxSelectAll_CheckedChanged);
            // 
            // buttonDelete
            // 
            this.buttonDelete.Location = new System.Drawing.Point(219, 478);
            this.buttonDelete.Name = "buttonDelete";
            this.buttonDelete.Size = new System.Drawing.Size(75, 23);
            this.buttonDelete.TabIndex = 25;
            this.buttonDelete.Text = "删除";
            this.buttonDelete.UseVisualStyleBackColor = true;
            this.buttonDelete.Click += new System.EventHandler(this.buttonDelete_Click);
            // 
            // buttonMm
            // 
            this.buttonMm.Location = new System.Drawing.Point(12, 478);
            this.buttonMm.Name = "buttonMm";
            this.buttonMm.Size = new System.Drawing.Size(75, 23);
            this.buttonMm.TabIndex = 26;
            this.buttonMm.Text = "微信";
            this.buttonMm.UseVisualStyleBackColor = true;
            this.buttonMm.Click += new System.EventHandler(this.buttonMm_Click);
            // 
            // buttonSelectAll
            // 
            this.buttonSelectAll.Location = new System.Drawing.Point(300, 449);
            this.buttonSelectAll.Name = "buttonSelectAll";
            this.buttonSelectAll.Size = new System.Drawing.Size(75, 23);
            this.buttonSelectAll.TabIndex = 27;
            this.buttonSelectAll.Text = "全选";
            this.buttonSelectAll.UseVisualStyleBackColor = true;
            this.buttonSelectAll.Click += new System.EventHandler(this.buttonSelectAll_Click);
            // 
            // buttonCopy
            // 
            this.buttonCopy.Location = new System.Drawing.Point(300, 393);
            this.buttonCopy.Name = "buttonCopy";
            this.buttonCopy.Size = new System.Drawing.Size(75, 23);
            this.buttonCopy.TabIndex = 28;
            this.buttonCopy.Text = "复制";
            this.buttonCopy.UseVisualStyleBackColor = true;
            this.buttonCopy.Click += new System.EventHandler(this.buttonCopy_Click);
            // 
            // buttonPaste
            // 
            this.buttonPaste.Location = new System.Drawing.Point(300, 422);
            this.buttonPaste.Name = "buttonPaste";
            this.buttonPaste.Size = new System.Drawing.Size(75, 23);
            this.buttonPaste.TabIndex = 29;
            this.buttonPaste.Text = "粘贴";
            this.buttonPaste.UseVisualStyleBackColor = true;
            this.buttonPaste.Click += new System.EventHandler(this.buttonPaste_Click);
            // 
            // buttonComplete
            // 
            this.buttonComplete.Location = new System.Drawing.Point(219, 449);
            this.buttonComplete.Name = "buttonComplete";
            this.buttonComplete.Size = new System.Drawing.Size(75, 23);
            this.buttonComplete.TabIndex = 30;
            this.buttonComplete.Text = "完成";
            this.buttonComplete.UseVisualStyleBackColor = true;
            this.buttonComplete.Click += new System.EventHandler(this.buttonComplete_Click);
            // 
            // FormNetworkIme
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(468, 513);
            this.Controls.Add(this.buttonComplete);
            this.Controls.Add(this.buttonPaste);
            this.Controls.Add(this.buttonCopy);
            this.Controls.Add(this.buttonSelectAll);
            this.Controls.Add(this.buttonMm);
            this.Controls.Add(this.buttonDelete);
            this.Controls.Add(this.checkBoxSelectAll);
            this.Controls.Add(this.checkBoxShareClipboard);
            this.Controls.Add(this.checkBoxEnterSend);
            this.Controls.Add(this.checkBoxRepeat);
            this.Controls.Add(this.labelStatus);
            this.Controls.Add(this.buttonEnter);
            this.Controls.Add(this.buttonRight);
            this.Controls.Add(this.buttonLeft);
            this.Controls.Add(this.buttonDown);
            this.Controls.Add(this.buttonUp);
            this.Controls.Add(this.buttonBack);
            this.Controls.Add(this.checkBoxClear);
            this.Controls.Add(this.groupBoxMode);
            this.Controls.Add(this.buttonClear);
            this.Controls.Add(this.buttonSend);
            this.Controls.Add(this.textBoxContent);
            this.Controls.Add(this.checkedListBoxClients);
            this.Controls.Add(this.buttonStart);
            this.Controls.Add(this.textBoxPort);
            this.Controls.Add(this.labelPort);
            this.Name = "FormNetworkIme";
            this.Text = "网络输入法";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.FormNetworkIme_FormClosing);
            this.groupBoxMode.ResumeLayout(false);
            this.groupBoxMode.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label labelPort;
        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.CheckedListBox checkedListBoxClients;
        private System.Windows.Forms.TextBox textBoxContent;
        private System.Windows.Forms.Button buttonSend;
        private System.Windows.Forms.Button buttonClear;
        private System.Windows.Forms.GroupBox groupBoxMode;
        private System.Windows.Forms.RadioButton radioButtonReplace;
        private System.Windows.Forms.RadioButton radioButtonInsert;
        private System.Windows.Forms.RadioButton radioButtonSend;
        private System.Windows.Forms.CheckBox checkBoxClear;
        private System.Windows.Forms.Button buttonBack;
        private System.Windows.Forms.Button buttonUp;
        private System.Windows.Forms.Button buttonDown;
        private System.Windows.Forms.Button buttonLeft;
        private System.Windows.Forms.Button buttonRight;
        private System.Windows.Forms.Button buttonEnter;
        private System.Windows.Forms.Label labelStatus;
        private System.Windows.Forms.CheckBox checkBoxRepeat;
        private System.Windows.Forms.CheckBox checkBoxEnterSend;
        private System.ComponentModel.BackgroundWorker backgroundWorkerRepeater;
        private System.Windows.Forms.CheckBox checkBoxShareClipboard;
        private System.Windows.Forms.CheckBox checkBoxSelectAll;
        private System.Windows.Forms.Button buttonDelete;
        private System.Windows.Forms.Button buttonMm;
        private System.Windows.Forms.Button buttonSelectAll;
        private System.Windows.Forms.Button buttonCopy;
        private System.Windows.Forms.Button buttonPaste;
        private System.Windows.Forms.Button buttonComplete;
    }
}

