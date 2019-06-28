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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormNetworkIme));
            this.textBoxPort = new System.Windows.Forms.TextBox();
            this.buttonStart = new System.Windows.Forms.Button();
            this.checkedListBoxClients = new System.Windows.Forms.CheckedListBox();
            this.contextMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItemAutoRun = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemNetworkImeAuto = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemWebProxyAuto = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemReverseProxyAuto = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemServers = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemWebProxy = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemTcpProxy = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemUrlBuilder = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemReverseProxy = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemPack = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemOpen = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemSelect = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemRequest = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemExit = new System.Windows.Forms.ToolStripMenuItem();
            this.textBoxContent = new System.Windows.Forms.TextBox();
            this.buttonSend = new System.Windows.Forms.Button();
            this.buttonClear = new System.Windows.Forms.Button();
            this.checkBoxClear = new System.Windows.Forms.CheckBox();
            this.labelStatus = new System.Windows.Forms.Label();
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
            this.comboBoxSend = new System.Windows.Forms.ComboBox();
            this.buttonShare = new System.Windows.Forms.Button();
            this.buttonShareFriends = new System.Windows.Forms.Button();
            this.buttonVolume = new System.Windows.Forms.Button();
            this.buttonMute = new System.Windows.Forms.Button();
            this.checkBoxAutoUnlock = new System.Windows.Forms.CheckBox();
            this.buttonLock = new System.Windows.Forms.Button();
            this.checkBoxFloatClock = new System.Windows.Forms.CheckBox();
            this.comboBoxRepeat = new System.Windows.Forms.ComboBox();
            this.checkBoxFakeShare = new System.Windows.Forms.CheckBox();
            this.contextMenuStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(292, 13);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(41, 21);
            this.textBoxPort.TabIndex = 1;
            this.textBoxPort.Text = "8865";
            // 
            // buttonStart
            // 
            this.buttonStart.Location = new System.Drawing.Point(336, 12);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(60, 23);
            this.buttonStart.TabIndex = 2;
            this.buttonStart.Text = "启动";
            this.buttonStart.UseVisualStyleBackColor = true;
            this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
            // 
            // checkedListBoxClients
            // 
            this.checkedListBoxClients.ContextMenuStrip = this.contextMenuStrip;
            this.checkedListBoxClients.FormattingEnabled = true;
            this.checkedListBoxClients.Location = new System.Drawing.Point(9, 41);
            this.checkedListBoxClients.Name = "checkedListBoxClients";
            this.checkedListBoxClients.Size = new System.Drawing.Size(390, 132);
            this.checkedListBoxClients.TabIndex = 4;
            this.checkedListBoxClients.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.checkedListBoxClients_ItemCheck);
            // 
            // contextMenuStrip
            // 
            this.contextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemAutoRun,
            this.toolStripMenuItemServers,
            this.toolStripMenuItemPack,
            this.toolStripMenuItemOpen,
            this.toolStripMenuItemSelect,
            this.toolStripMenuItemRequest,
            this.toolStripMenuItemExit});
            this.contextMenuStrip.Name = "contextMenuStrip";
            this.contextMenuStrip.Size = new System.Drawing.Size(125, 158);
            this.contextMenuStrip.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenuStrip_Opening);
            // 
            // toolStripMenuItemAutoRun
            // 
            this.toolStripMenuItemAutoRun.CheckOnClick = true;
            this.toolStripMenuItemAutoRun.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemNetworkImeAuto,
            this.toolStripMenuItemWebProxyAuto,
            this.toolStripMenuItemReverseProxyAuto});
            this.toolStripMenuItemAutoRun.Name = "toolStripMenuItemAutoRun";
            this.toolStripMenuItemAutoRun.Size = new System.Drawing.Size(124, 22);
            this.toolStripMenuItemAutoRun.Text = "开机启动";
            this.toolStripMenuItemAutoRun.DropDownOpening += new System.EventHandler(this.toolStripMenuItemAutoRun_DropDownOpening);
            this.toolStripMenuItemAutoRun.Click += new System.EventHandler(this.toolStripMenuItemAutoRun_Click);
            // 
            // toolStripMenuItemNetworkImeAuto
            // 
            this.toolStripMenuItemNetworkImeAuto.CheckOnClick = true;
            this.toolStripMenuItemNetworkImeAuto.Name = "toolStripMenuItemNetworkImeAuto";
            this.toolStripMenuItemNetworkImeAuto.Size = new System.Drawing.Size(152, 22);
            this.toolStripMenuItemNetworkImeAuto.Text = "网络输入法";
            this.toolStripMenuItemNetworkImeAuto.Click += new System.EventHandler(this.toolStripMenuItemNetworkImeAuto_Click);
            // 
            // toolStripMenuItemWebProxyAuto
            // 
            this.toolStripMenuItemWebProxyAuto.CheckOnClick = true;
            this.toolStripMenuItemWebProxyAuto.Name = "toolStripMenuItemWebProxyAuto";
            this.toolStripMenuItemWebProxyAuto.Size = new System.Drawing.Size(152, 22);
            this.toolStripMenuItemWebProxyAuto.Text = "Web 代理";
            this.toolStripMenuItemWebProxyAuto.Click += new System.EventHandler(this.toolStripMenuItemWebProxyAuto_Click);
            // 
            // toolStripMenuItemReverseProxyAuto
            // 
            this.toolStripMenuItemReverseProxyAuto.CheckOnClick = true;
            this.toolStripMenuItemReverseProxyAuto.Name = "toolStripMenuItemReverseProxyAuto";
            this.toolStripMenuItemReverseProxyAuto.Size = new System.Drawing.Size(152, 22);
            this.toolStripMenuItemReverseProxyAuto.Text = "反向代理";
            this.toolStripMenuItemReverseProxyAuto.Click += new System.EventHandler(this.toolStripMenuItemReverseProxyAuto_Click);
            // 
            // toolStripMenuItemServers
            // 
            this.toolStripMenuItemServers.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemWebProxy,
            this.toolStripMenuItemTcpProxy,
            this.toolStripMenuItemUrlBuilder,
            this.toolStripMenuItemReverseProxy});
            this.toolStripMenuItemServers.Name = "toolStripMenuItemServers";
            this.toolStripMenuItemServers.Size = new System.Drawing.Size(124, 22);
            this.toolStripMenuItemServers.Text = "服务器";
            // 
            // toolStripMenuItemWebProxy
            // 
            this.toolStripMenuItemWebProxy.Name = "toolStripMenuItemWebProxy";
            this.toolStripMenuItemWebProxy.Size = new System.Drawing.Size(131, 22);
            this.toolStripMenuItemWebProxy.Text = "Web 代理";
            this.toolStripMenuItemWebProxy.Click += new System.EventHandler(this.toolStripMenuItemWebProxy_Click);
            // 
            // toolStripMenuItemTcpProxy
            // 
            this.toolStripMenuItemTcpProxy.Name = "toolStripMenuItemTcpProxy";
            this.toolStripMenuItemTcpProxy.Size = new System.Drawing.Size(131, 22);
            this.toolStripMenuItemTcpProxy.Text = "Tcp 代理";
            this.toolStripMenuItemTcpProxy.Click += new System.EventHandler(this.toolStripMenuItemTcpProxy_Click);
            // 
            // toolStripMenuItemUrlBuilder
            // 
            this.toolStripMenuItemUrlBuilder.Name = "toolStripMenuItemUrlBuilder";
            this.toolStripMenuItemUrlBuilder.Size = new System.Drawing.Size(131, 22);
            this.toolStripMenuItemUrlBuilder.Text = "Url 服务";
            this.toolStripMenuItemUrlBuilder.Click += new System.EventHandler(this.toolStripMenuItemUrlBuilder_Click);
            // 
            // toolStripMenuItemReverseProxy
            // 
            this.toolStripMenuItemReverseProxy.Name = "toolStripMenuItemReverseProxy";
            this.toolStripMenuItemReverseProxy.Size = new System.Drawing.Size(131, 22);
            this.toolStripMenuItemReverseProxy.Text = "反向代理";
            this.toolStripMenuItemReverseProxy.Click += new System.EventHandler(this.toolStripMenuItemReverseProxy_Click);
            // 
            // toolStripMenuItemPack
            // 
            this.toolStripMenuItemPack.Name = "toolStripMenuItemPack";
            this.toolStripMenuItemPack.Size = new System.Drawing.Size(124, 22);
            this.toolStripMenuItemPack.Text = "组包";
            this.toolStripMenuItemPack.Click += new System.EventHandler(this.buttonBuildPack_Click);
            // 
            // toolStripMenuItemOpen
            // 
            this.toolStripMenuItemOpen.Name = "toolStripMenuItemOpen";
            this.toolStripMenuItemOpen.Size = new System.Drawing.Size(124, 22);
            this.toolStripMenuItemOpen.Text = "打开";
            this.toolStripMenuItemOpen.Click += new System.EventHandler(this.buttonOpen_Click);
            // 
            // toolStripMenuItemSelect
            // 
            this.toolStripMenuItemSelect.Name = "toolStripMenuItemSelect";
            this.toolStripMenuItemSelect.Size = new System.Drawing.Size(124, 22);
            this.toolStripMenuItemSelect.Text = "选择";
            this.toolStripMenuItemSelect.Click += new System.EventHandler(this.buttonSelect_Click);
            // 
            // toolStripMenuItemRequest
            // 
            this.toolStripMenuItemRequest.Name = "toolStripMenuItemRequest";
            this.toolStripMenuItemRequest.Size = new System.Drawing.Size(124, 22);
            this.toolStripMenuItemRequest.Text = "请求";
            this.toolStripMenuItemRequest.Click += new System.EventHandler(this.buttonRequest_Click);
            // 
            // toolStripMenuItemExit
            // 
            this.toolStripMenuItemExit.Name = "toolStripMenuItemExit";
            this.toolStripMenuItemExit.Size = new System.Drawing.Size(124, 22);
            this.toolStripMenuItemExit.Text = "退出";
            this.toolStripMenuItemExit.Click += new System.EventHandler(this.toolStripMenuItemExit_Click);
            // 
            // textBoxContent
            // 
            this.textBoxContent.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
            this.textBoxContent.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.HistoryList;
            this.textBoxContent.Location = new System.Drawing.Point(10, 199);
            this.textBoxContent.Multiline = true;
            this.textBoxContent.Name = "textBoxContent";
            this.textBoxContent.Size = new System.Drawing.Size(389, 69);
            this.textBoxContent.TabIndex = 5;
            // 
            // buttonSend
            // 
            this.buttonSend.Location = new System.Drawing.Point(339, 441);
            this.buttonSend.Name = "buttonSend";
            this.buttonSend.Size = new System.Drawing.Size(60, 23);
            this.buttonSend.TabIndex = 6;
            this.buttonSend.Text = "发送";
            this.buttonSend.UseVisualStyleBackColor = true;
            this.buttonSend.Click += new System.EventHandler(this.buttonSend_Click);
            // 
            // buttonClear
            // 
            this.buttonClear.Location = new System.Drawing.Point(273, 383);
            this.buttonClear.Name = "buttonClear";
            this.buttonClear.Size = new System.Drawing.Size(60, 23);
            this.buttonClear.TabIndex = 7;
            this.buttonClear.Text = "清理";
            this.buttonClear.UseVisualStyleBackColor = true;
            this.buttonClear.Click += new System.EventHandler(this.buttonClear_Click);
            // 
            // checkBoxClear
            // 
            this.checkBoxClear.AutoSize = true;
            this.checkBoxClear.Location = new System.Drawing.Point(225, 303);
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
            // checkBoxEnterSend
            // 
            this.checkBoxEnterSend.AutoSize = true;
            this.checkBoxEnterSend.Checked = true;
            this.checkBoxEnterSend.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxEnterSend.Location = new System.Drawing.Point(315, 303);
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
            this.checkBoxShareClipboard.Location = new System.Drawing.Point(225, 177);
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
            this.buttonTmall.Location = new System.Drawing.Point(141, 354);
            this.buttonTmall.Name = "buttonTmall";
            this.buttonTmall.Size = new System.Drawing.Size(60, 23);
            this.buttonTmall.TabIndex = 28;
            this.buttonTmall.Text = "天猫";
            this.buttonTmall.UseVisualStyleBackColor = true;
            this.buttonTmall.Click += new System.EventHandler(this.buttonTmall_Click);
            // 
            // buttonPaste
            // 
            this.buttonPaste.Location = new System.Drawing.Point(339, 412);
            this.buttonPaste.Name = "buttonPaste";
            this.buttonPaste.Size = new System.Drawing.Size(60, 23);
            this.buttonPaste.TabIndex = 29;
            this.buttonPaste.Text = "粘贴";
            this.buttonPaste.UseVisualStyleBackColor = true;
            this.buttonPaste.Click += new System.EventHandler(this.buttonPaste_Click);
            // 
            // buttonComplete
            // 
            this.buttonComplete.Location = new System.Drawing.Point(339, 383);
            this.buttonComplete.Name = "buttonComplete";
            this.buttonComplete.Size = new System.Drawing.Size(60, 23);
            this.buttonComplete.TabIndex = 30;
            this.buttonComplete.Text = "完成";
            this.buttonComplete.UseVisualStyleBackColor = true;
            this.buttonComplete.Click += new System.EventHandler(this.buttonComplete_Click);
            // 
            // buttonOpen
            // 
            this.buttonOpen.Location = new System.Drawing.Point(141, 325);
            this.buttonOpen.Name = "buttonOpen";
            this.buttonOpen.Size = new System.Drawing.Size(60, 23);
            this.buttonOpen.TabIndex = 31;
            this.buttonOpen.Text = "打开";
            this.buttonOpen.UseVisualStyleBackColor = true;
            this.buttonOpen.Click += new System.EventHandler(this.buttonOpen_Click);
            // 
            // buttonBack
            // 
            this.buttonBack.Location = new System.Drawing.Point(141, 441);
            this.buttonBack.Name = "buttonBack";
            this.buttonBack.Size = new System.Drawing.Size(60, 23);
            this.buttonBack.TabIndex = 33;
            this.buttonBack.Text = "返回";
            this.buttonBack.UseVisualStyleBackColor = true;
            this.buttonBack.Click += new System.EventHandler(this.buttonBack_Click);
            // 
            // buttonVolumeDown
            // 
            this.buttonVolumeDown.Location = new System.Drawing.Point(207, 354);
            this.buttonVolumeDown.Name = "buttonVolumeDown";
            this.buttonVolumeDown.Size = new System.Drawing.Size(60, 23);
            this.buttonVolumeDown.TabIndex = 34;
            this.buttonVolumeDown.Text = "音量-";
            this.buttonVolumeDown.UseVisualStyleBackColor = true;
            this.buttonVolumeDown.Click += new System.EventHandler(this.buttonVolumeDown_Click);
            // 
            // buttonVolumeUp
            // 
            this.buttonVolumeUp.Location = new System.Drawing.Point(339, 354);
            this.buttonVolumeUp.Name = "buttonVolumeUp";
            this.buttonVolumeUp.Size = new System.Drawing.Size(60, 23);
            this.buttonVolumeUp.TabIndex = 35;
            this.buttonVolumeUp.Text = "音量+";
            this.buttonVolumeUp.UseVisualStyleBackColor = true;
            this.buttonVolumeUp.Click += new System.EventHandler(this.buttonVolumeUp_Click);
            // 
            // buttonHome
            // 
            this.buttonHome.Location = new System.Drawing.Point(207, 441);
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
            this.buttonRequest.Location = new System.Drawing.Point(273, 325);
            this.buttonRequest.Name = "buttonRequest";
            this.buttonRequest.Size = new System.Drawing.Size(60, 23);
            this.buttonRequest.TabIndex = 40;
            this.buttonRequest.Text = "请求";
            this.buttonRequest.UseVisualStyleBackColor = true;
            this.buttonRequest.Click += new System.EventHandler(this.buttonRequest_Click);
            // 
            // buttonHidden
            // 
            this.buttonHidden.Location = new System.Drawing.Point(339, 325);
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
            // buttonRecents
            // 
            this.buttonRecents.Location = new System.Drawing.Point(273, 441);
            this.buttonRecents.Name = "buttonRecents";
            this.buttonRecents.Size = new System.Drawing.Size(60, 23);
            this.buttonRecents.TabIndex = 42;
            this.buttonRecents.Text = "任务";
            this.buttonRecents.UseVisualStyleBackColor = true;
            this.buttonRecents.Click += new System.EventHandler(this.buttonRecents_Click);
            // 
            // buttonRefresh
            // 
            this.buttonRefresh.Location = new System.Drawing.Point(141, 383);
            this.buttonRefresh.Name = "buttonRefresh";
            this.buttonRefresh.Size = new System.Drawing.Size(60, 23);
            this.buttonRefresh.TabIndex = 43;
            this.buttonRefresh.Text = "刷新";
            this.buttonRefresh.UseVisualStyleBackColor = true;
            this.buttonRefresh.Click += new System.EventHandler(this.buttonRefresh_Click);
            // 
            // buttonSignin
            // 
            this.buttonSignin.Location = new System.Drawing.Point(141, 412);
            this.buttonSignin.Name = "buttonSignin";
            this.buttonSignin.Size = new System.Drawing.Size(60, 23);
            this.buttonSignin.TabIndex = 44;
            this.buttonSignin.Text = "签到";
            this.buttonSignin.UseVisualStyleBackColor = true;
            this.buttonSignin.Click += new System.EventHandler(this.buttonSignin_Click);
            // 
            // buttonUnfollow
            // 
            this.buttonUnfollow.Location = new System.Drawing.Point(207, 412);
            this.buttonUnfollow.Name = "buttonUnfollow";
            this.buttonUnfollow.Size = new System.Drawing.Size(60, 23);
            this.buttonUnfollow.TabIndex = 45;
            this.buttonUnfollow.Text = "取关";
            this.buttonUnfollow.UseVisualStyleBackColor = true;
            this.buttonUnfollow.Click += new System.EventHandler(this.buttonUnfollow_Click);
            // 
            // textBoxClock
            // 
            this.textBoxClock.Location = new System.Drawing.Point(118, 13);
            this.textBoxClock.Name = "textBoxClock";
            this.textBoxClock.ReadOnly = true;
            this.textBoxClock.Size = new System.Drawing.Size(168, 21);
            this.textBoxClock.TabIndex = 46;
            // 
            // buttonBuildPack
            // 
            this.buttonBuildPack.Location = new System.Drawing.Point(207, 325);
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
            this.comboBoxHistory.Size = new System.Drawing.Size(321, 20);
            this.comboBoxHistory.TabIndex = 48;
            this.comboBoxHistory.TextChanged += new System.EventHandler(this.comboBoxHistory_TextChanged);
            // 
            // buttonWeixin
            // 
            this.buttonWeixin.Location = new System.Drawing.Point(9, 441);
            this.buttonWeixin.Name = "buttonWeixin";
            this.buttonWeixin.Size = new System.Drawing.Size(60, 23);
            this.buttonWeixin.TabIndex = 49;
            this.buttonWeixin.Text = "微信";
            this.buttonWeixin.UseVisualStyleBackColor = true;
            this.buttonWeixin.Click += new System.EventHandler(this.buttonWeixin_Click);
            // 
            // buttonCommand
            // 
            this.buttonCommand.Location = new System.Drawing.Point(75, 325);
            this.buttonCommand.Name = "buttonCommand";
            this.buttonCommand.Size = new System.Drawing.Size(60, 23);
            this.buttonCommand.TabIndex = 50;
            this.buttonCommand.Text = "命令";
            this.buttonCommand.UseVisualStyleBackColor = true;
            this.buttonCommand.Click += new System.EventHandler(this.buttonCommand_Click);
            // 
            // buttonUnlock
            // 
            this.buttonUnlock.Location = new System.Drawing.Point(75, 441);
            this.buttonUnlock.Name = "buttonUnlock";
            this.buttonUnlock.Size = new System.Drawing.Size(60, 23);
            this.buttonUnlock.TabIndex = 51;
            this.buttonUnlock.Text = "解锁";
            this.buttonUnlock.UseVisualStyleBackColor = true;
            this.buttonUnlock.Click += new System.EventHandler(this.buttonUnlock_Click);
            // 
            // buttonIme
            // 
            this.buttonIme.Location = new System.Drawing.Point(9, 354);
            this.buttonIme.Name = "buttonIme";
            this.buttonIme.Size = new System.Drawing.Size(60, 23);
            this.buttonIme.TabIndex = 52;
            this.buttonIme.Text = "输入法";
            this.buttonIme.UseVisualStyleBackColor = true;
            this.buttonIme.Click += new System.EventHandler(this.buttonIme_Click);
            // 
            // buttonDesktop
            // 
            this.buttonDesktop.Location = new System.Drawing.Point(273, 412);
            this.buttonDesktop.Name = "buttonDesktop";
            this.buttonDesktop.Size = new System.Drawing.Size(60, 23);
            this.buttonDesktop.TabIndex = 56;
            this.buttonDesktop.Text = "桌面";
            this.buttonDesktop.UseVisualStyleBackColor = true;
            this.buttonDesktop.Click += new System.EventHandler(this.buttonDesktop_Click);
            // 
            // buttonAlipay
            // 
            this.buttonAlipay.Location = new System.Drawing.Point(9, 325);
            this.buttonAlipay.Name = "buttonAlipay";
            this.buttonAlipay.Size = new System.Drawing.Size(60, 23);
            this.buttonAlipay.TabIndex = 55;
            this.buttonAlipay.Text = "支付宝";
            this.buttonAlipay.UseVisualStyleBackColor = true;
            this.buttonAlipay.Click += new System.EventHandler(this.buttonAlipay_Click);
            // 
            // buttonWeibo
            // 
            this.buttonWeibo.Location = new System.Drawing.Point(9, 412);
            this.buttonWeibo.Name = "buttonWeibo";
            this.buttonWeibo.Size = new System.Drawing.Size(60, 23);
            this.buttonWeibo.TabIndex = 54;
            this.buttonWeibo.Text = "微博";
            this.buttonWeibo.UseVisualStyleBackColor = true;
            this.buttonWeibo.Click += new System.EventHandler(this.buttonWeibo_Click);
            // 
            // buttonTaobao
            // 
            this.buttonTaobao.Location = new System.Drawing.Point(75, 354);
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
            this.checkBoxSendClipboard.Location = new System.Drawing.Point(315, 177);
            this.checkBoxSendClipboard.Name = "checkBoxSendClipboard";
            this.checkBoxSendClipboard.Size = new System.Drawing.Size(84, 16);
            this.checkBoxSendClipboard.TabIndex = 57;
            this.checkBoxSendClipboard.Text = "剪切板发送";
            this.checkBoxSendClipboard.UseVisualStyleBackColor = true;
            // 
            // comboBoxSend
            // 
            this.comboBoxSend.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxSend.FormattingEnabled = true;
            this.comboBoxSend.Items.AddRange(new object[] {
            "发送",
            "替换",
            "插入"});
            this.comboBoxSend.Location = new System.Drawing.Point(339, 274);
            this.comboBoxSend.Name = "comboBoxSend";
            this.comboBoxSend.Size = new System.Drawing.Size(60, 20);
            this.comboBoxSend.TabIndex = 58;
            // 
            // buttonShare
            // 
            this.buttonShare.Location = new System.Drawing.Point(75, 383);
            this.buttonShare.Name = "buttonShare";
            this.buttonShare.Size = new System.Drawing.Size(60, 23);
            this.buttonShare.TabIndex = 59;
            this.buttonShare.Text = "分圈";
            this.buttonShare.UseVisualStyleBackColor = true;
            this.buttonShare.Click += new System.EventHandler(this.buttonShare_Click);
            // 
            // buttonShareFriends
            // 
            this.buttonShareFriends.Location = new System.Drawing.Point(9, 383);
            this.buttonShareFriends.Name = "buttonShareFriends";
            this.buttonShareFriends.Size = new System.Drawing.Size(60, 23);
            this.buttonShareFriends.TabIndex = 60;
            this.buttonShareFriends.Text = "分朋友";
            this.buttonShareFriends.UseVisualStyleBackColor = true;
            this.buttonShareFriends.Click += new System.EventHandler(this.buttonShareFriends_Click);
            // 
            // buttonVolume
            // 
            this.buttonVolume.Location = new System.Drawing.Point(273, 354);
            this.buttonVolume.Name = "buttonVolume";
            this.buttonVolume.Size = new System.Drawing.Size(60, 23);
            this.buttonVolume.TabIndex = 61;
            this.buttonVolume.Text = "音量";
            this.buttonVolume.UseVisualStyleBackColor = true;
            this.buttonVolume.Click += new System.EventHandler(this.buttonVolume_Click);
            // 
            // buttonMute
            // 
            this.buttonMute.Location = new System.Drawing.Point(207, 383);
            this.buttonMute.Name = "buttonMute";
            this.buttonMute.Size = new System.Drawing.Size(60, 23);
            this.buttonMute.TabIndex = 63;
            this.buttonMute.Text = "静音";
            this.buttonMute.UseVisualStyleBackColor = true;
            this.buttonMute.Click += new System.EventHandler(this.buttonMute_Click);
            // 
            // checkBoxAutoUnlock
            // 
            this.checkBoxAutoUnlock.AutoSize = true;
            this.checkBoxAutoUnlock.Checked = true;
            this.checkBoxAutoUnlock.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxAutoUnlock.Location = new System.Drawing.Point(147, 303);
            this.checkBoxAutoUnlock.Name = "checkBoxAutoUnlock";
            this.checkBoxAutoUnlock.Size = new System.Drawing.Size(72, 16);
            this.checkBoxAutoUnlock.TabIndex = 64;
            this.checkBoxAutoUnlock.Text = "自动解锁";
            this.checkBoxAutoUnlock.UseVisualStyleBackColor = true;
            // 
            // buttonLock
            // 
            this.buttonLock.Location = new System.Drawing.Point(75, 412);
            this.buttonLock.Name = "buttonLock";
            this.buttonLock.Size = new System.Drawing.Size(60, 23);
            this.buttonLock.TabIndex = 65;
            this.buttonLock.Text = "锁屏";
            this.buttonLock.UseVisualStyleBackColor = true;
            this.buttonLock.Click += new System.EventHandler(this.buttonLock_Click);
            // 
            // checkBoxFloatClock
            // 
            this.checkBoxFloatClock.AutoSize = true;
            this.checkBoxFloatClock.Checked = true;
            this.checkBoxFloatClock.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxFloatClock.Location = new System.Drawing.Point(147, 177);
            this.checkBoxFloatClock.Name = "checkBoxFloatClock";
            this.checkBoxFloatClock.Size = new System.Drawing.Size(72, 16);
            this.checkBoxFloatClock.TabIndex = 66;
            this.checkBoxFloatClock.Text = "悬浮时钟";
            this.checkBoxFloatClock.UseVisualStyleBackColor = true;
            this.checkBoxFloatClock.CheckedChanged += new System.EventHandler(this.checkBoxFloatClock_CheckedChanged);
            // 
            // comboBoxRepeat
            // 
            this.comboBoxRepeat.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxRepeat.FormattingEnabled = true;
            this.comboBoxRepeat.Items.AddRange(new object[] {
            "不重复发送",
            "间隔100毫秒",
            "间隔200毫秒",
            "间隔300毫秒",
            "间隔400毫秒",
            "间隔500毫秒",
            "间隔600毫秒",
            "间隔700毫秒",
            "间隔800毫秒",
            "间隔900毫秒",
            "间隔1000毫秒",
            "间隔1100毫秒",
            "间隔1200毫秒",
            "间隔1300毫秒",
            "间隔1400毫秒",
            "间隔1500毫秒",
            "间隔1600毫秒",
            "间隔1700毫秒",
            "间隔1800毫秒",
            "间隔1900毫秒",
            "间隔2000毫秒",
            "间隔2100毫秒",
            "间隔2200毫秒",
            "间隔2300毫秒",
            "间隔2400毫秒",
            "间隔2500毫秒",
            "间隔2600毫秒",
            "间隔2700毫秒",
            "间隔2800毫秒",
            "间隔2900毫秒",
            "间隔3000毫秒",
            "间隔3100毫秒",
            "间隔3200毫秒",
            "间隔3300毫秒",
            "间隔3400毫秒",
            "间隔3500毫秒",
            "间隔3600毫秒",
            "间隔3700毫秒",
            "间隔3800毫秒",
            "间隔3900毫秒",
            "间隔4000毫秒",
            "间隔4100毫秒",
            "间隔4200毫秒",
            "间隔4300毫秒",
            "间隔4400毫秒",
            "间隔4500毫秒",
            "间隔4600毫秒",
            "间隔4700毫秒",
            "间隔4800毫秒",
            "间隔4900毫秒",
            "间隔5000毫秒"});
            this.comboBoxRepeat.Location = new System.Drawing.Point(9, 13);
            this.comboBoxRepeat.Name = "comboBoxRepeat";
            this.comboBoxRepeat.Size = new System.Drawing.Size(103, 20);
            this.comboBoxRepeat.TabIndex = 67;
            this.comboBoxRepeat.SelectedIndexChanged += new System.EventHandler(this.comboBoxRepeat_SelectedIndexChanged);
            // 
            // checkBoxFakeShare
            // 
            this.checkBoxFakeShare.AutoSize = true;
            this.checkBoxFakeShare.Location = new System.Drawing.Point(93, 177);
            this.checkBoxFakeShare.Name = "checkBoxFakeShare";
            this.checkBoxFakeShare.Size = new System.Drawing.Size(48, 16);
            this.checkBoxFakeShare.TabIndex = 68;
            this.checkBoxFakeShare.Text = "假分";
            this.checkBoxFakeShare.UseVisualStyleBackColor = true;
            // 
            // FormNetworkIme
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(408, 474);
            this.ContextMenuStrip = this.contextMenuStrip;
            this.Controls.Add(this.checkBoxFakeShare);
            this.Controls.Add(this.comboBoxRepeat);
            this.Controls.Add(this.checkBoxFloatClock);
            this.Controls.Add(this.buttonLock);
            this.Controls.Add(this.checkBoxAutoUnlock);
            this.Controls.Add(this.buttonMute);
            this.Controls.Add(this.buttonVolume);
            this.Controls.Add(this.buttonShareFriends);
            this.Controls.Add(this.buttonShare);
            this.Controls.Add(this.comboBoxSend);
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
            this.Controls.Add(this.labelStatus);
            this.Controls.Add(this.checkBoxClear);
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
            this.Load += new System.EventHandler(this.FormNetworkIme_Load);
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
        private System.Windows.Forms.CheckBox checkBoxClear;
        private System.Windows.Forms.Label labelStatus;
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
        private System.Windows.Forms.ComboBox comboBoxSend;
        private System.Windows.Forms.Button buttonShare;
        private System.Windows.Forms.Button buttonShareFriends;
        private System.Windows.Forms.Button buttonVolume;
        private System.Windows.Forms.Button buttonMute;
        private System.Windows.Forms.CheckBox checkBoxAutoUnlock;
        private System.Windows.Forms.Button buttonLock;
        private System.Windows.Forms.CheckBox checkBoxFloatClock;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemAutoRun;
        private System.Windows.Forms.ComboBox comboBoxRepeat;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemServers;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemWebProxy;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemTcpProxy;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemUrlBuilder;
        private System.Windows.Forms.CheckBox checkBoxFakeShare;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemNetworkImeAuto;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemWebProxyAuto;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemReverseProxy;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemReverseProxyAuto;
    }
}

