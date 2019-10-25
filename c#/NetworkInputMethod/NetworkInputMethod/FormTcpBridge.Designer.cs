namespace NetworkInputMethod
{
    partial class FormTcpBridge
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
            this.textBoxUrl1 = new System.Windows.Forms.TextBox();
            this.textBoxUrl2 = new System.Windows.Forms.TextBox();
            this.buttonAdd = new System.Windows.Forms.Button();
            this.listViewBridges = new System.Windows.Forms.ListView();
            this.columnHeaderState = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderUrl1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderUrl2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.contextMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItemStart = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemStop = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemRemove = new System.Windows.Forms.ToolStripMenuItem();
            this.contextMenuStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // textBoxUrl1
            // 
            this.textBoxUrl1.Location = new System.Drawing.Point(12, 11);
            this.textBoxUrl1.Name = "textBoxUrl1";
            this.textBoxUrl1.Size = new System.Drawing.Size(311, 21);
            this.textBoxUrl1.TabIndex = 0;
            this.textBoxUrl1.Text = "192.168.88.1:8864";
            // 
            // textBoxUrl2
            // 
            this.textBoxUrl2.Location = new System.Drawing.Point(329, 11);
            this.textBoxUrl2.Name = "textBoxUrl2";
            this.textBoxUrl2.Size = new System.Drawing.Size(311, 21);
            this.textBoxUrl2.TabIndex = 1;
            this.textBoxUrl2.Text = "127.0.0.1:8864";
            // 
            // buttonAdd
            // 
            this.buttonAdd.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonAdd.Location = new System.Drawing.Point(697, 10);
            this.buttonAdd.Name = "buttonAdd";
            this.buttonAdd.Size = new System.Drawing.Size(75, 23);
            this.buttonAdd.TabIndex = 2;
            this.buttonAdd.Text = "添加";
            this.buttonAdd.UseVisualStyleBackColor = true;
            this.buttonAdd.Click += new System.EventHandler(this.buttonAdd_Click);
            // 
            // listViewBridges
            // 
            this.listViewBridges.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.listViewBridges.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderState,
            this.columnHeaderUrl1,
            this.columnHeaderUrl2});
            this.listViewBridges.ContextMenuStrip = this.contextMenuStrip;
            this.listViewBridges.FullRowSelect = true;
            this.listViewBridges.GridLines = true;
            this.listViewBridges.HideSelection = false;
            this.listViewBridges.Location = new System.Drawing.Point(12, 39);
            this.listViewBridges.Name = "listViewBridges";
            this.listViewBridges.Size = new System.Drawing.Size(760, 510);
            this.listViewBridges.TabIndex = 3;
            this.listViewBridges.UseCompatibleStateImageBehavior = false;
            this.listViewBridges.View = System.Windows.Forms.View.Details;
            // 
            // columnHeaderState
            // 
            this.columnHeaderState.Text = "状态";
            this.columnHeaderState.Width = 80;
            // 
            // columnHeaderUrl1
            // 
            this.columnHeaderUrl1.Text = "URL1";
            this.columnHeaderUrl1.Width = 320;
            // 
            // columnHeaderUrl2
            // 
            this.columnHeaderUrl2.Text = "URL2";
            this.columnHeaderUrl2.Width = 320;
            // 
            // contextMenuStrip
            // 
            this.contextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemStart,
            this.toolStripMenuItemStop,
            this.toolStripMenuItemRemove});
            this.contextMenuStrip.Name = "contextMenuStrip";
            this.contextMenuStrip.Size = new System.Drawing.Size(101, 70);
            // 
            // toolStripMenuItemStart
            // 
            this.toolStripMenuItemStart.Name = "toolStripMenuItemStart";
            this.toolStripMenuItemStart.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemStart.Text = "启动";
            this.toolStripMenuItemStart.Click += new System.EventHandler(this.toolStripMenuItemStart_Click);
            // 
            // toolStripMenuItemStop
            // 
            this.toolStripMenuItemStop.Name = "toolStripMenuItemStop";
            this.toolStripMenuItemStop.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemStop.Text = "停止";
            this.toolStripMenuItemStop.Click += new System.EventHandler(this.toolStripMenuItemStop_Click);
            // 
            // toolStripMenuItemRemove
            // 
            this.toolStripMenuItemRemove.Name = "toolStripMenuItemRemove";
            this.toolStripMenuItemRemove.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemRemove.Text = "删除";
            this.toolStripMenuItemRemove.Click += new System.EventHandler(this.toolStripMenuItemRemove_Click);
            // 
            // FormTcpBridge
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(784, 561);
            this.Controls.Add(this.listViewBridges);
            this.Controls.Add(this.buttonAdd);
            this.Controls.Add(this.textBoxUrl2);
            this.Controls.Add(this.textBoxUrl1);
            this.Name = "FormTcpBridge";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "网络桥接";
            this.Load += new System.EventHandler(this.FormTcpBridge_Load);
            this.contextMenuStrip.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox textBoxUrl1;
        private System.Windows.Forms.TextBox textBoxUrl2;
        private System.Windows.Forms.Button buttonAdd;
        private System.Windows.Forms.ListView listViewBridges;
        private System.Windows.Forms.ColumnHeader columnHeaderState;
        private System.Windows.Forms.ColumnHeader columnHeaderUrl1;
        private System.Windows.Forms.ColumnHeader columnHeaderUrl2;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemStart;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemStop;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemRemove;
    }
}