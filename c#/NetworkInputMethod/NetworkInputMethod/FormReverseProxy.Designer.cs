namespace NetworkInputMethod
{
    partial class FormReverseProxy
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
            this.listViewProxys = new System.Windows.Forms.ListView();
            this.columnHeaderProxyState = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderProxyLinkCount = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderProxyLocalPort = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderTarget = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderServer = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderClientAddress = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderClientName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.contextMenuStripProxy = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItemProxyStart = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemProxyStop = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemProxyAdd = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemProxyRemove = new System.Windows.Forms.ToolStripMenuItem();
            this.listViewClients = new System.Windows.Forms.ListView();
            this.columnHeaderLinkCount = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderAddress = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderHostname = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.contextMenuStripClient = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItemAddProxy = new System.Windows.Forms.ToolStripMenuItem();
            this.buttonSwitch = new System.Windows.Forms.Button();
            this.textBoxPort = new System.Windows.Forms.TextBox();
            this.timerKeepAlive = new System.Windows.Forms.Timer(this.components);
            this.textBoxLinkCount = new System.Windows.Forms.TextBox();
            this.groupBoxService = new System.Windows.Forms.GroupBox();
            this.groupBoxClient = new System.Windows.Forms.GroupBox();
            this.textBoxSlaveCount = new System.Windows.Forms.TextBox();
            this.buttonClientStart = new System.Windows.Forms.Button();
            this.textBoxClientUrl = new System.Windows.Forms.TextBox();
            this.contextMenuStripProxy.SuspendLayout();
            this.contextMenuStripClient.SuspendLayout();
            this.groupBoxService.SuspendLayout();
            this.groupBoxClient.SuspendLayout();
            this.SuspendLayout();
            // 
            // listViewProxys
            // 
            this.listViewProxys.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderProxyState,
            this.columnHeaderProxyLinkCount,
            this.columnHeaderProxyLocalPort,
            this.columnHeaderTarget,
            this.columnHeaderServer,
            this.columnHeaderClientAddress,
            this.columnHeaderClientName});
            this.listViewProxys.ContextMenuStrip = this.contextMenuStripProxy;
            this.listViewProxys.FullRowSelect = true;
            this.listViewProxys.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.listViewProxys.LabelWrap = false;
            this.listViewProxys.Location = new System.Drawing.Point(12, 377);
            this.listViewProxys.Name = "listViewProxys";
            this.listViewProxys.Size = new System.Drawing.Size(760, 372);
            this.listViewProxys.TabIndex = 3;
            this.listViewProxys.UseCompatibleStateImageBehavior = false;
            this.listViewProxys.View = System.Windows.Forms.View.Details;
            // 
            // columnHeaderProxyState
            // 
            this.columnHeaderProxyState.Text = "状态";
            // 
            // columnHeaderProxyLinkCount
            // 
            this.columnHeaderProxyLinkCount.Text = "连接数";
            this.columnHeaderProxyLinkCount.Width = 50;
            // 
            // columnHeaderProxyLocalPort
            // 
            this.columnHeaderProxyLocalPort.Text = "本地端口";
            // 
            // columnHeaderTarget
            // 
            this.columnHeaderTarget.Text = "目标地址";
            this.columnHeaderTarget.Width = 145;
            // 
            // columnHeaderServer
            // 
            this.columnHeaderServer.Text = "服务器地址";
            this.columnHeaderServer.Width = 145;
            // 
            // columnHeaderClientAddress
            // 
            this.columnHeaderClientAddress.Text = "客户端地址";
            this.columnHeaderClientAddress.Width = 110;
            // 
            // columnHeaderClientName
            // 
            this.columnHeaderClientName.Text = "客户端名称";
            this.columnHeaderClientName.Width = 186;
            // 
            // contextMenuStripProxy
            // 
            this.contextMenuStripProxy.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemProxyStart,
            this.toolStripMenuItemProxyStop,
            this.toolStripMenuItemProxyAdd,
            this.toolStripMenuItemProxyRemove});
            this.contextMenuStripProxy.Name = "contextMenuStripProxy";
            this.contextMenuStripProxy.Size = new System.Drawing.Size(101, 92);
            // 
            // toolStripMenuItemProxyStart
            // 
            this.toolStripMenuItemProxyStart.Name = "toolStripMenuItemProxyStart";
            this.toolStripMenuItemProxyStart.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemProxyStart.Text = "启动";
            this.toolStripMenuItemProxyStart.Click += new System.EventHandler(this.toolStripMenuItemProxyStart_Click);
            // 
            // toolStripMenuItemProxyStop
            // 
            this.toolStripMenuItemProxyStop.Name = "toolStripMenuItemProxyStop";
            this.toolStripMenuItemProxyStop.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemProxyStop.Text = "停止";
            this.toolStripMenuItemProxyStop.Click += new System.EventHandler(this.toolStripMenuItemProxyStop_Click);
            // 
            // toolStripMenuItemProxyAdd
            // 
            this.toolStripMenuItemProxyAdd.Name = "toolStripMenuItemProxyAdd";
            this.toolStripMenuItemProxyAdd.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemProxyAdd.Text = "添加";
            this.toolStripMenuItemProxyAdd.Click += new System.EventHandler(this.toolStripMenuItemProxyAdd_Click);
            // 
            // toolStripMenuItemProxyRemove
            // 
            this.toolStripMenuItemProxyRemove.Name = "toolStripMenuItemProxyRemove";
            this.toolStripMenuItemProxyRemove.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemProxyRemove.Text = "删除";
            this.toolStripMenuItemProxyRemove.Click += new System.EventHandler(this.toolStripMenuItemProxyRemove_Click);
            // 
            // listViewClients
            // 
            this.listViewClients.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderLinkCount,
            this.columnHeaderAddress,
            this.columnHeaderHostname});
            this.listViewClients.ContextMenuStrip = this.contextMenuStripClient;
            this.listViewClients.FullRowSelect = true;
            this.listViewClients.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.listViewClients.HideSelection = false;
            this.listViewClients.LabelWrap = false;
            this.listViewClients.Location = new System.Drawing.Point(12, 81);
            this.listViewClients.Name = "listViewClients";
            this.listViewClients.Size = new System.Drawing.Size(760, 290);
            this.listViewClients.TabIndex = 2;
            this.listViewClients.UseCompatibleStateImageBehavior = false;
            this.listViewClients.View = System.Windows.Forms.View.Details;
            // 
            // columnHeaderLinkCount
            // 
            this.columnHeaderLinkCount.Text = "连接数";
            this.columnHeaderLinkCount.Width = 50;
            // 
            // columnHeaderAddress
            // 
            this.columnHeaderAddress.Text = "客户端地址";
            this.columnHeaderAddress.Width = 110;
            // 
            // columnHeaderHostname
            // 
            this.columnHeaderHostname.Text = "客户端名称";
            this.columnHeaderHostname.Width = 596;
            // 
            // contextMenuStripClient
            // 
            this.contextMenuStripClient.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemAddProxy});
            this.contextMenuStripClient.Name = "contextMenuStripClient";
            this.contextMenuStripClient.Size = new System.Drawing.Size(101, 26);
            // 
            // toolStripMenuItemAddProxy
            // 
            this.toolStripMenuItemAddProxy.Name = "toolStripMenuItemAddProxy";
            this.toolStripMenuItemAddProxy.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemAddProxy.Text = "映射";
            this.toolStripMenuItemAddProxy.Click += new System.EventHandler(this.toolStripMenuItemAddProxy_Click);
            // 
            // buttonSwitch
            // 
            this.buttonSwitch.Location = new System.Drawing.Point(110, 19);
            this.buttonSwitch.Name = "buttonSwitch";
            this.buttonSwitch.Size = new System.Drawing.Size(75, 23);
            this.buttonSwitch.TabIndex = 1;
            this.buttonSwitch.Text = "启动";
            this.buttonSwitch.UseVisualStyleBackColor = true;
            this.buttonSwitch.Click += new System.EventHandler(this.buttonSwitch_Click);
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(6, 20);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(42, 21);
            this.textBoxPort.TabIndex = 0;
            this.textBoxPort.Text = "8868";
            // 
            // timerKeepAlive
            // 
            this.timerKeepAlive.Enabled = true;
            this.timerKeepAlive.Interval = 10000;
            this.timerKeepAlive.Tick += new System.EventHandler(this.timerKeepAlive_Tick);
            // 
            // textBoxLinkCount
            // 
            this.textBoxLinkCount.Location = new System.Drawing.Point(54, 20);
            this.textBoxLinkCount.Name = "textBoxLinkCount";
            this.textBoxLinkCount.ReadOnly = true;
            this.textBoxLinkCount.Size = new System.Drawing.Size(50, 21);
            this.textBoxLinkCount.TabIndex = 4;
            this.textBoxLinkCount.Text = "0";
            // 
            // groupBoxService
            // 
            this.groupBoxService.AutoSize = true;
            this.groupBoxService.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.groupBoxService.Controls.Add(this.textBoxPort);
            this.groupBoxService.Controls.Add(this.textBoxLinkCount);
            this.groupBoxService.Controls.Add(this.buttonSwitch);
            this.groupBoxService.Location = new System.Drawing.Point(12, 12);
            this.groupBoxService.Name = "groupBoxService";
            this.groupBoxService.Size = new System.Drawing.Size(191, 62);
            this.groupBoxService.TabIndex = 5;
            this.groupBoxService.TabStop = false;
            this.groupBoxService.Text = "服务器";
            // 
            // groupBoxClient
            // 
            this.groupBoxClient.AutoSize = true;
            this.groupBoxClient.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.groupBoxClient.Controls.Add(this.textBoxSlaveCount);
            this.groupBoxClient.Controls.Add(this.buttonClientStart);
            this.groupBoxClient.Controls.Add(this.textBoxClientUrl);
            this.groupBoxClient.Location = new System.Drawing.Point(221, 12);
            this.groupBoxClient.Name = "groupBoxClient";
            this.groupBoxClient.Size = new System.Drawing.Size(551, 62);
            this.groupBoxClient.TabIndex = 6;
            this.groupBoxClient.TabStop = false;
            this.groupBoxClient.Text = "客户端";
            // 
            // textBoxSlaveCount
            // 
            this.textBoxSlaveCount.Location = new System.Drawing.Point(414, 20);
            this.textBoxSlaveCount.Name = "textBoxSlaveCount";
            this.textBoxSlaveCount.ReadOnly = true;
            this.textBoxSlaveCount.Size = new System.Drawing.Size(50, 21);
            this.textBoxSlaveCount.TabIndex = 2;
            this.textBoxSlaveCount.Text = "0";
            // 
            // buttonClientStart
            // 
            this.buttonClientStart.Location = new System.Drawing.Point(470, 19);
            this.buttonClientStart.Name = "buttonClientStart";
            this.buttonClientStart.Size = new System.Drawing.Size(75, 23);
            this.buttonClientStart.TabIndex = 1;
            this.buttonClientStart.Text = "启动";
            this.buttonClientStart.UseVisualStyleBackColor = true;
            this.buttonClientStart.Click += new System.EventHandler(this.ButtonClientStart_Click);
            // 
            // textBoxClientUrl
            // 
            this.textBoxClientUrl.Location = new System.Drawing.Point(6, 20);
            this.textBoxClientUrl.Name = "textBoxClientUrl";
            this.textBoxClientUrl.Size = new System.Drawing.Size(402, 21);
            this.textBoxClientUrl.TabIndex = 0;
            // 
            // FormReverseProxy
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(784, 761);
            this.Controls.Add(this.groupBoxClient);
            this.Controls.Add(this.groupBoxService);
            this.Controls.Add(this.listViewProxys);
            this.Controls.Add(this.listViewClients);
            this.Name = "FormReverseProxy";
            this.Text = "反向代理";
            this.Load += new System.EventHandler(this.FormReverseProxy_Load);
            this.contextMenuStripProxy.ResumeLayout(false);
            this.contextMenuStripClient.ResumeLayout(false);
            this.groupBoxService.ResumeLayout(false);
            this.groupBoxService.PerformLayout();
            this.groupBoxClient.ResumeLayout(false);
            this.groupBoxClient.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Button buttonSwitch;
        private System.Windows.Forms.ListView listViewClients;
        private System.Windows.Forms.ListView listViewProxys;
        private System.Windows.Forms.ColumnHeader columnHeaderLinkCount;
        private System.Windows.Forms.ColumnHeader columnHeaderHostname;
        private System.Windows.Forms.ColumnHeader columnHeaderProxyLocalPort;
        private System.Windows.Forms.ColumnHeader columnHeaderServer;
        private System.Windows.Forms.ColumnHeader columnHeaderClientAddress;
        private System.Windows.Forms.ColumnHeader columnHeaderTarget;
        private System.Windows.Forms.ContextMenuStrip contextMenuStripProxy;
        private System.Windows.Forms.ContextMenuStrip contextMenuStripClient;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemAddProxy;
        private System.Windows.Forms.ColumnHeader columnHeaderAddress;
        private System.Windows.Forms.ColumnHeader columnHeaderClientName;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemProxyStart;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemProxyStop;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemProxyAdd;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemProxyRemove;
        private System.Windows.Forms.ColumnHeader columnHeaderProxyState;
        private System.Windows.Forms.Timer timerKeepAlive;
        private System.Windows.Forms.ColumnHeader columnHeaderProxyLinkCount;
        private System.Windows.Forms.TextBox textBoxLinkCount;
        private System.Windows.Forms.GroupBox groupBoxService;
        private System.Windows.Forms.GroupBox groupBoxClient;
        private System.Windows.Forms.Button buttonClientStart;
        private System.Windows.Forms.TextBox textBoxClientUrl;
        private System.Windows.Forms.TextBox textBoxSlaveCount;
    }
}