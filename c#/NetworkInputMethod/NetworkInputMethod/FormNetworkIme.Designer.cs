﻿namespace NetworkInputMethod
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormNetworkIme));
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
            this.labelStatus = new System.Windows.Forms.Label();
            this.checkBoxRepeat = new System.Windows.Forms.CheckBox();
            this.checkBoxEnterSend = new System.Windows.Forms.CheckBox();
            this.backgroundWorkerRepeater = new System.ComponentModel.BackgroundWorker();
            this.checkBoxShareClipboard = new System.Windows.Forms.CheckBox();
            this.checkBoxSelectAll = new System.Windows.Forms.CheckBox();
            this.buttonTmall = new System.Windows.Forms.Button();
            this.buttonPaste = new System.Windows.Forms.Button();
            this.buttonComplete = new System.Windows.Forms.Button();
            this.buttonOpen = new System.Windows.Forms.Button();
            this.buttonBack = new System.Windows.Forms.Button();
            this.buttonVolumeDown = new System.Windows.Forms.Button();
            this.buttonVolumeUp = new System.Windows.Forms.Button();
            this.buttonHome = new System.Windows.Forms.Button();
            this.timerKeepAlive = new System.Windows.Forms.Timer(this.components);
            this.buttonRequest = new System.Windows.Forms.Button();
            this.buttonHidden = new System.Windows.Forms.Button();
            this.notifyIcon = new System.Windows.Forms.NotifyIcon(this.components);
            this.contextMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItemPack = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemOpen = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemSelect = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemRequest = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemExit = new System.Windows.Forms.ToolStripMenuItem();
            this.buttonRecents = new System.Windows.Forms.Button();
            this.buttonRefresh = new System.Windows.Forms.Button();
            this.buttonSignin = new System.Windows.Forms.Button();
            this.buttonUnfollow = new System.Windows.Forms.Button();
            this.textBoxClock = new System.Windows.Forms.TextBox();
            this.buttonBuildPack = new System.Windows.Forms.Button();
            this.comboBoxHistory = new System.Windows.Forms.ComboBox();
            this.buttonWeixin = new System.Windows.Forms.Button();
            this.buttonCommand = new System.Windows.Forms.Button();
            this.buttonUnlock = new System.Windows.Forms.Button();
            this.buttonIme = new System.Windows.Forms.Button();
            this.buttonDesktop = new System.Windows.Forms.Button();
            this.buttonAlipay = new System.Windows.Forms.Button();
            this.buttonWeibo = new System.Windows.Forms.Button();
            this.buttonTaobao = new System.Windows.Forms.Button();
            this.checkBoxSendClipboard = new System.Windows.Forms.CheckBox();
            this.groupBoxMode.SuspendLayout();
            this.contextMenuStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(311, 14);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(75, 21);
            this.textBoxPort.TabIndex = 1;
            this.textBoxPort.Text = "8865";
            // 
            // buttonStart
            // 
            this.buttonStart.Location = new System.Drawing.Point(392, 12);
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
            this.checkedListBoxClients.Location = new System.Drawing.Point(12, 41);
            this.checkedListBoxClients.Name = "checkedListBoxClients";
            this.checkedListBoxClients.Size = new System.Drawing.Size(455, 132);
            this.checkedListBoxClients.TabIndex = 4;
            this.checkedListBoxClients.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.checkedListBoxClients_ItemCheck);
            // 
            // textBoxContent
            // 
            this.textBoxContent.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.textBoxContent.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.HistoryList;
            this.textBoxContent.Location = new System.Drawing.Point(12, 199);
            this.textBoxContent.Multiline = true;
            this.textBoxContent.Name = "textBoxContent";
            this.textBoxContent.Size = new System.Drawing.Size(455, 69);
            this.textBoxContent.TabIndex = 5;
            // 
            // buttonSend
            // 
            this.buttonSend.Location = new System.Drawing.Point(407, 424);
            this.buttonSend.Name = "buttonSend";
            this.buttonSend.Size = new System.Drawing.Size(60, 23);
            this.buttonSend.TabIndex = 6;
            this.buttonSend.Text = "发送";
            this.buttonSend.UseVisualStyleBackColor = true;
            this.buttonSend.Click += new System.EventHandler(this.buttonSend_Click);
            // 
            // buttonClear
            // 
            this.buttonClear.Location = new System.Drawing.Point(275, 395);
            this.buttonClear.Name = "buttonClear";
            this.buttonClear.Size = new System.Drawing.Size(60, 23);
            this.buttonClear.TabIndex = 7;
            this.buttonClear.Text = "清理";
            this.buttonClear.UseVisualStyleBackColor = true;
            this.buttonClear.Click += new System.EventHandler(this.buttonClear_Click);
            // 
            // groupBoxMode
            // 
            this.groupBoxMode.Controls.Add(this.radioButtonReplace);
            this.groupBoxMode.Controls.Add(this.radioButtonInsert);
            this.groupBoxMode.Controls.Add(this.radioButtonSend);
            this.groupBoxMode.Location = new System.Drawing.Point(407, 325);
            this.groupBoxMode.Name = "groupBoxMode";
            this.groupBoxMode.Size = new System.Drawing.Size(60, 94);
            this.groupBoxMode.TabIndex = 8;
            this.groupBoxMode.TabStop = false;
            // 
            // radioButtonReplace
            // 
            this.radioButtonReplace.AutoSize = true;
            this.radioButtonReplace.Location = new System.Drawing.Point(6, 44);
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
            this.radioButtonInsert.Location = new System.Drawing.Point(7, 73);
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
            this.radioButtonSend.Location = new System.Drawing.Point(6, 15);
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
            this.checkBoxClear.Location = new System.Drawing.Point(305, 303);
            this.checkBoxClear.Name = "checkBoxClear";
            this.checkBoxClear.Size = new System.Drawing.Size(84, 16);
            this.checkBoxClear.TabIndex = 9;
            this.checkBoxClear.Text = "发送后清除";
            this.checkBoxClear.UseVisualStyleBackColor = true;
            // 
            // labelStatus
            // 
            this.labelStatus.AutoSize = true;
            this.labelStatus.Location = new System.Drawing.Point(10, 304);
            this.labelStatus.Name = "labelStatus";
            this.labelStatus.Size = new System.Drawing.Size(77, 12);
            this.labelStatus.TabIndex = 20;
            this.labelStatus.Text = "服务器未启动";
            // 
            // checkBoxRepeat
            // 
            this.checkBoxRepeat.AutoSize = true;
            this.checkBoxRepeat.Location = new System.Drawing.Point(395, 177);
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
            this.checkBoxEnterSend.Location = new System.Drawing.Point(395, 303);
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
            this.checkBoxShareClipboard.Location = new System.Drawing.Point(215, 177);
            this.checkBoxShareClipboard.Name = "checkBoxShareClipboard";
            this.checkBoxShareClipboard.Size = new System.Drawing.Size(84, 16);
            this.checkBoxShareClipboard.TabIndex = 23;
            this.checkBoxShareClipboard.Text = "剪切板分享";
            this.checkBoxShareClipboard.UseVisualStyleBackColor = true;
            // 
            // checkBoxSelectAll
            // 
            this.checkBoxSelectAll.AutoSize = true;
            this.checkBoxSelectAll.Location = new System.Drawing.Point(11, 177);
            this.checkBoxSelectAll.Name = "checkBoxSelectAll";
            this.checkBoxSelectAll.Size = new System.Drawing.Size(48, 16);
            this.checkBoxSelectAll.TabIndex = 24;
            this.checkBoxSelectAll.Text = "全选";
            this.checkBoxSelectAll.UseVisualStyleBackColor = true;
            this.checkBoxSelectAll.CheckedChanged += new System.EventHandler(this.checkBoxSelectAll_CheckedChanged);
            // 
            // buttonTmall
            // 
            this.buttonTmall.Location = new System.Drawing.Point(209, 366);
            this.buttonTmall.Name = "buttonTmall";
            this.buttonTmall.Size = new System.Drawing.Size(60, 23);
            this.buttonTmall.TabIndex = 28;
            this.buttonTmall.Text = "天猫";
            this.buttonTmall.UseVisualStyleBackColor = true;
            this.buttonTmall.Click += new System.EventHandler(this.buttonTmall_Click);
            // 
            // buttonPaste
            // 
            this.buttonPaste.Location = new System.Drawing.Point(341, 395);
            this.buttonPaste.Name = "buttonPaste";
            this.buttonPaste.Size = new System.Drawing.Size(60, 23);
            this.buttonPaste.TabIndex = 29;
            this.buttonPaste.Text = "粘贴";
            this.buttonPaste.UseVisualStyleBackColor = true;
            this.buttonPaste.Click += new System.EventHandler(this.buttonPaste_Click);
            // 
            // buttonComplete
            // 
            this.buttonComplete.Location = new System.Drawing.Point(341, 424);
            this.buttonComplete.Name = "buttonComplete";
            this.buttonComplete.Size = new System.Drawing.Size(60, 23);
            this.buttonComplete.TabIndex = 30;
            this.buttonComplete.Text = "完成";
            this.buttonComplete.UseVisualStyleBackColor = true;
            this.buttonComplete.Click += new System.EventHandler(this.buttonComplete_Click);
            // 
            // buttonOpen
            // 
            this.buttonOpen.Location = new System.Drawing.Point(209, 337);
            this.buttonOpen.Name = "buttonOpen";
            this.buttonOpen.Size = new System.Drawing.Size(60, 23);
            this.buttonOpen.TabIndex = 31;
            this.buttonOpen.Text = "打开";
            this.buttonOpen.UseVisualStyleBackColor = true;
            this.buttonOpen.Click += new System.EventHandler(this.buttonOpen_Click);
            // 
            // buttonBack
            // 
            this.buttonBack.Location = new System.Drawing.Point(143, 424);
            this.buttonBack.Name = "buttonBack";
            this.buttonBack.Size = new System.Drawing.Size(60, 23);
            this.buttonBack.TabIndex = 33;
            this.buttonBack.Text = "返回";
            this.buttonBack.UseVisualStyleBackColor = true;
            this.buttonBack.Click += new System.EventHandler(this.buttonBack_Click);
            // 
            // buttonVolumeDown
            // 
            this.buttonVolumeDown.Location = new System.Drawing.Point(275, 366);
            this.buttonVolumeDown.Name = "buttonVolumeDown";
            this.buttonVolumeDown.Size = new System.Drawing.Size(60, 23);
            this.buttonVolumeDown.TabIndex = 34;
            this.buttonVolumeDown.Text = "音量-";
            this.buttonVolumeDown.UseVisualStyleBackColor = true;
            this.buttonVolumeDown.Click += new System.EventHandler(this.buttonVolumeDown_Click);
            // 
            // buttonVolumeUp
            // 
            this.buttonVolumeUp.Location = new System.Drawing.Point(341, 366);
            this.buttonVolumeUp.Name = "buttonVolumeUp";
            this.buttonVolumeUp.Size = new System.Drawing.Size(60, 23);
            this.buttonVolumeUp.TabIndex = 35;
            this.buttonVolumeUp.Text = "音量+";
            this.buttonVolumeUp.UseVisualStyleBackColor = true;
            this.buttonVolumeUp.Click += new System.EventHandler(this.buttonVolumeUp_Click);
            // 
            // buttonHome
            // 
            this.buttonHome.Location = new System.Drawing.Point(209, 424);
            this.buttonHome.Name = "buttonHome";
            this.buttonHome.Size = new System.Drawing.Size(60, 23);
            this.buttonHome.TabIndex = 37;
            this.buttonHome.Text = "主页";
            this.buttonHome.UseVisualStyleBackColor = true;
            this.buttonHome.Click += new System.EventHandler(this.buttonHome_Click);
            // 
            // timerKeepAlive
            // 
            this.timerKeepAlive.Enabled = true;
            this.timerKeepAlive.Interval = 60000;
            this.timerKeepAlive.Tick += new System.EventHandler(this.timerKeepAlive_Tick);
            // 
            // buttonRequest
            // 
            this.buttonRequest.Location = new System.Drawing.Point(275, 337);
            this.buttonRequest.Name = "buttonRequest";
            this.buttonRequest.Size = new System.Drawing.Size(60, 23);
            this.buttonRequest.TabIndex = 40;
            this.buttonRequest.Text = "请求";
            this.buttonRequest.UseVisualStyleBackColor = true;
            this.buttonRequest.Click += new System.EventHandler(this.buttonRequest_Click);
            // 
            // buttonHidden
            // 
            this.buttonHidden.Location = new System.Drawing.Point(77, 366);
            this.buttonHidden.Name = "buttonHidden";
            this.buttonHidden.Size = new System.Drawing.Size(60, 23);
            this.buttonHidden.TabIndex = 41;
            this.buttonHidden.Text = "隐藏";
            this.buttonHidden.UseVisualStyleBackColor = true;
            // 
            // notifyIcon
            // 
            this.notifyIcon.ContextMenuStrip = this.contextMenuStrip;
            this.notifyIcon.Icon = ((System.Drawing.Icon)(resources.GetObject("notifyIcon.Icon")));
            this.notifyIcon.Text = "网络输入法";
            this.notifyIcon.Visible = true;
            this.notifyIcon.MouseClick += new System.Windows.Forms.MouseEventHandler(this.notifyIcon_MouseClick);
            // 
            // contextMenuStrip
            // 
            this.contextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemPack,
            this.toolStripMenuItemOpen,
            this.toolStripMenuItemSelect,
            this.toolStripMenuItemRequest,
            this.toolStripMenuItemExit});
            this.contextMenuStrip.Name = "contextMenuStrip";
            this.contextMenuStrip.Size = new System.Drawing.Size(101, 114);
            // 
            // toolStripMenuItemPack
            // 
            this.toolStripMenuItemPack.Name = "toolStripMenuItemPack";
            this.toolStripMenuItemPack.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemPack.Text = "组包";
            this.toolStripMenuItemPack.Click += new System.EventHandler(this.buttonBuildPack_Click);
            // 
            // toolStripMenuItemOpen
            // 
            this.toolStripMenuItemOpen.Name = "toolStripMenuItemOpen";
            this.toolStripMenuItemOpen.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemOpen.Text = "打开";
            this.toolStripMenuItemOpen.Click += new System.EventHandler(this.buttonOpen_Click);
            // 
            // toolStripMenuItemSelect
            // 
            this.toolStripMenuItemSelect.Name = "toolStripMenuItemSelect";
            this.toolStripMenuItemSelect.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemSelect.Text = "选择";
            this.toolStripMenuItemSelect.Click += new System.EventHandler(this.buttonSelect_Click);
            // 
            // toolStripMenuItemRequest
            // 
            this.toolStripMenuItemRequest.Name = "toolStripMenuItemRequest";
            this.toolStripMenuItemRequest.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemRequest.Text = "请求";
            this.toolStripMenuItemRequest.Click += new System.EventHandler(this.buttonRequest_Click);
            // 
            // toolStripMenuItemExit
            // 
            this.toolStripMenuItemExit.Name = "toolStripMenuItemExit";
            this.toolStripMenuItemExit.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemExit.Text = "退出";
            this.toolStripMenuItemExit.Click += new System.EventHandler(this.toolStripMenuItemExit_Click);
            // 
            // buttonRecents
            // 
            this.buttonRecents.Location = new System.Drawing.Point(275, 424);
            this.buttonRecents.Name = "buttonRecents";
            this.buttonRecents.Size = new System.Drawing.Size(60, 23);
            this.buttonRecents.TabIndex = 42;
            this.buttonRecents.Text = "任务";
            this.buttonRecents.UseVisualStyleBackColor = true;
            this.buttonRecents.Click += new System.EventHandler(this.buttonRecents_Click);
            // 
            // buttonRefresh
            // 
            this.buttonRefresh.Location = new System.Drawing.Point(143, 395);
            this.buttonRefresh.Name = "buttonRefresh";
            this.buttonRefresh.Size = new System.Drawing.Size(60, 23);
            this.buttonRefresh.TabIndex = 43;
            this.buttonRefresh.Text = "刷新";
            this.buttonRefresh.UseVisualStyleBackColor = true;
            this.buttonRefresh.Click += new System.EventHandler(this.buttonRefresh_Click);
            // 
            // buttonSignin
            // 
            this.buttonSignin.Location = new System.Drawing.Point(143, 337);
            this.buttonSignin.Name = "buttonSignin";
            this.buttonSignin.Size = new System.Drawing.Size(60, 23);
            this.buttonSignin.TabIndex = 44;
            this.buttonSignin.Text = "签到";
            this.buttonSignin.UseVisualStyleBackColor = true;
            this.buttonSignin.Click += new System.EventHandler(this.buttonSignin_Click);
            // 
            // buttonUnfollow
            // 
            this.buttonUnfollow.Location = new System.Drawing.Point(77, 395);
            this.buttonUnfollow.Name = "buttonUnfollow";
            this.buttonUnfollow.Size = new System.Drawing.Size(60, 23);
            this.buttonUnfollow.TabIndex = 45;
            this.buttonUnfollow.Text = "取关";
            this.buttonUnfollow.UseVisualStyleBackColor = true;
            this.buttonUnfollow.Click += new System.EventHandler(this.buttonUnfollow_Click);
            // 
            // textBoxClock
            // 
            this.textBoxClock.Location = new System.Drawing.Point(12, 14);
            this.textBoxClock.Name = "textBoxClock";
            this.textBoxClock.ReadOnly = true;
            this.textBoxClock.Size = new System.Drawing.Size(293, 21);
            this.textBoxClock.TabIndex = 46;
            // 
            // buttonBuildPack
            // 
            this.buttonBuildPack.Location = new System.Drawing.Point(341, 337);
            this.buttonBuildPack.Name = "buttonBuildPack";
            this.buttonBuildPack.Size = new System.Drawing.Size(60, 23);
            this.buttonBuildPack.TabIndex = 47;
            this.buttonBuildPack.Text = "组包";
            this.buttonBuildPack.UseVisualStyleBackColor = true;
            this.buttonBuildPack.Click += new System.EventHandler(this.buttonBuildPack_Click);
            // 
            // comboBoxHistory
            // 
            this.comboBoxHistory.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxHistory.FormattingEnabled = true;
            this.comboBoxHistory.Location = new System.Drawing.Point(12, 274);
            this.comboBoxHistory.Name = "comboBoxHistory";
            this.comboBoxHistory.Size = new System.Drawing.Size(455, 20);
            this.comboBoxHistory.TabIndex = 48;
            this.comboBoxHistory.TextChanged += new System.EventHandler(this.comboBoxHistory_TextChanged);
            // 
            // buttonWeixin
            // 
            this.buttonWeixin.Location = new System.Drawing.Point(11, 424);
            this.buttonWeixin.Name = "buttonWeixin";
            this.buttonWeixin.Size = new System.Drawing.Size(60, 23);
            this.buttonWeixin.TabIndex = 49;
            this.buttonWeixin.Text = "微信";
            this.buttonWeixin.UseVisualStyleBackColor = true;
            this.buttonWeixin.Click += new System.EventHandler(this.buttonWeixin_Click);
            // 
            // buttonCommand
            // 
            this.buttonCommand.Location = new System.Drawing.Point(77, 337);
            this.buttonCommand.Name = "buttonCommand";
            this.buttonCommand.Size = new System.Drawing.Size(60, 23);
            this.buttonCommand.TabIndex = 50;
            this.buttonCommand.Text = "命令";
            this.buttonCommand.UseVisualStyleBackColor = true;
            this.buttonCommand.Click += new System.EventHandler(this.buttonCommand_Click);
            // 
            // buttonUnlock
            // 
            this.buttonUnlock.Location = new System.Drawing.Point(77, 424);
            this.buttonUnlock.Name = "buttonUnlock";
            this.buttonUnlock.Size = new System.Drawing.Size(60, 23);
            this.buttonUnlock.TabIndex = 51;
            this.buttonUnlock.Text = "解锁";
            this.buttonUnlock.UseVisualStyleBackColor = true;
            this.buttonUnlock.Click += new System.EventHandler(this.buttonUnlock_Click);
            // 
            // buttonIme
            // 
            this.buttonIme.Location = new System.Drawing.Point(11, 366);
            this.buttonIme.Name = "buttonIme";
            this.buttonIme.Size = new System.Drawing.Size(60, 23);
            this.buttonIme.TabIndex = 52;
            this.buttonIme.Text = "输入法";
            this.buttonIme.UseVisualStyleBackColor = true;
            this.buttonIme.Click += new System.EventHandler(this.buttonIme_Click);
            // 
            // buttonDesktop
            // 
            this.buttonDesktop.Location = new System.Drawing.Point(209, 395);
            this.buttonDesktop.Name = "buttonDesktop";
            this.buttonDesktop.Size = new System.Drawing.Size(60, 23);
            this.buttonDesktop.TabIndex = 56;
            this.buttonDesktop.Text = "桌面";
            this.buttonDesktop.UseVisualStyleBackColor = true;
            this.buttonDesktop.Click += new System.EventHandler(this.buttonDesktop_Click);
            // 
            // buttonAlipay
            // 
            this.buttonAlipay.Location = new System.Drawing.Point(11, 337);
            this.buttonAlipay.Name = "buttonAlipay";
            this.buttonAlipay.Size = new System.Drawing.Size(60, 23);
            this.buttonAlipay.TabIndex = 55;
            this.buttonAlipay.Text = "支付宝";
            this.buttonAlipay.UseVisualStyleBackColor = true;
            this.buttonAlipay.Click += new System.EventHandler(this.buttonAlipay_Click);
            // 
            // buttonWeibo
            // 
            this.buttonWeibo.Location = new System.Drawing.Point(11, 395);
            this.buttonWeibo.Name = "buttonWeibo";
            this.buttonWeibo.Size = new System.Drawing.Size(60, 23);
            this.buttonWeibo.TabIndex = 54;
            this.buttonWeibo.Text = "微博";
            this.buttonWeibo.UseVisualStyleBackColor = true;
            this.buttonWeibo.Click += new System.EventHandler(this.buttonWeibo_Click);
            // 
            // buttonTaobao
            // 
            this.buttonTaobao.Location = new System.Drawing.Point(143, 366);
            this.buttonTaobao.Name = "buttonTaobao";
            this.buttonTaobao.Size = new System.Drawing.Size(60, 23);
            this.buttonTaobao.TabIndex = 53;
            this.buttonTaobao.Text = "淘宝";
            this.buttonTaobao.UseVisualStyleBackColor = true;
            this.buttonTaobao.Click += new System.EventHandler(this.buttonTaobao_Click);
            // 
            // checkBoxSendClipboard
            // 
            this.checkBoxSendClipboard.AutoSize = true;
            this.checkBoxSendClipboard.Location = new System.Drawing.Point(305, 177);
            this.checkBoxSendClipboard.Name = "checkBoxSendClipboard";
            this.checkBoxSendClipboard.Size = new System.Drawing.Size(84, 16);
            this.checkBoxSendClipboard.TabIndex = 57;
            this.checkBoxSendClipboard.Text = "剪切板发送";
            this.checkBoxSendClipboard.UseVisualStyleBackColor = true;
            // 
            // FormNetworkIme
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(480, 457);
            this.Controls.Add(this.checkBoxSendClipboard);
            this.Controls.Add(this.buttonDesktop);
            this.Controls.Add(this.buttonAlipay);
            this.Controls.Add(this.buttonWeibo);
            this.Controls.Add(this.buttonTaobao);
            this.Controls.Add(this.buttonIme);
            this.Controls.Add(this.buttonUnlock);
            this.Controls.Add(this.buttonCommand);
            this.Controls.Add(this.buttonWeixin);
            this.Controls.Add(this.comboBoxHistory);
            this.Controls.Add(this.buttonBuildPack);
            this.Controls.Add(this.textBoxClock);
            this.Controls.Add(this.buttonUnfollow);
            this.Controls.Add(this.buttonSignin);
            this.Controls.Add(this.buttonRefresh);
            this.Controls.Add(this.buttonRecents);
            this.Controls.Add(this.buttonHidden);
            this.Controls.Add(this.buttonRequest);
            this.Controls.Add(this.buttonHome);
            this.Controls.Add(this.buttonVolumeUp);
            this.Controls.Add(this.buttonVolumeDown);
            this.Controls.Add(this.buttonBack);
            this.Controls.Add(this.buttonOpen);
            this.Controls.Add(this.buttonComplete);
            this.Controls.Add(this.buttonPaste);
            this.Controls.Add(this.buttonTmall);
            this.Controls.Add(this.checkBoxSelectAll);
            this.Controls.Add(this.checkBoxShareClipboard);
            this.Controls.Add(this.checkBoxEnterSend);
            this.Controls.Add(this.checkBoxRepeat);
            this.Controls.Add(this.labelStatus);
            this.Controls.Add(this.checkBoxClear);
            this.Controls.Add(this.groupBoxMode);
            this.Controls.Add(this.buttonClear);
            this.Controls.Add(this.buttonSend);
            this.Controls.Add(this.textBoxContent);
            this.Controls.Add(this.checkedListBoxClients);
            this.Controls.Add(this.buttonStart);
            this.Controls.Add(this.textBoxPort);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "FormNetworkIme";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "网络输入法";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.FormNetworkIme_FormClosing);
            this.groupBoxMode.ResumeLayout(false);
            this.groupBoxMode.PerformLayout();
            this.contextMenuStrip.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
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
        private System.Windows.Forms.Label labelStatus;
        private System.Windows.Forms.CheckBox checkBoxRepeat;
        private System.Windows.Forms.CheckBox checkBoxEnterSend;
        private System.ComponentModel.BackgroundWorker backgroundWorkerRepeater;
        private System.Windows.Forms.CheckBox checkBoxShareClipboard;
        private System.Windows.Forms.CheckBox checkBoxSelectAll;
        private System.Windows.Forms.Button buttonTmall;
        private System.Windows.Forms.Button buttonPaste;
        private System.Windows.Forms.Button buttonComplete;
        private System.Windows.Forms.Button buttonOpen;
        private System.Windows.Forms.Button buttonBack;
        private System.Windows.Forms.Button buttonVolumeDown;
        private System.Windows.Forms.Button buttonVolumeUp;
        private System.Windows.Forms.Button buttonHome;
        private System.Windows.Forms.Timer timerKeepAlive;
        private System.Windows.Forms.Button buttonRequest;
        private System.Windows.Forms.Button buttonHidden;
        private System.Windows.Forms.NotifyIcon notifyIcon;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemExit;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemRequest;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemOpen;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemSelect;
        private System.Windows.Forms.Button buttonRecents;
        private System.Windows.Forms.Button buttonRefresh;
        private System.Windows.Forms.Button buttonSignin;
        private System.Windows.Forms.Button buttonUnfollow;
        private System.Windows.Forms.TextBox textBoxClock;
        private System.Windows.Forms.Button buttonBuildPack;
        private System.Windows.Forms.ComboBox comboBoxHistory;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemPack;
        private System.Windows.Forms.Button buttonWeixin;
        private System.Windows.Forms.Button buttonCommand;
        private System.Windows.Forms.Button buttonUnlock;
        private System.Windows.Forms.Button buttonIme;
        private System.Windows.Forms.Button buttonDesktop;
        private System.Windows.Forms.Button buttonAlipay;
        private System.Windows.Forms.Button buttonWeibo;
        private System.Windows.Forms.Button buttonTaobao;
        private System.Windows.Forms.CheckBox checkBoxSendClipboard;
    }
}

