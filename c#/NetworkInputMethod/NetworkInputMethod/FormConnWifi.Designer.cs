namespace NetworkInputMethod
{
    partial class FormConnWifi
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
            this.listBoxWifis = new System.Windows.Forms.ListBox();
            this.contextMenuStripWifis = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItemConn = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemAdd = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemDel = new System.Windows.Forms.ToolStripMenuItem();
            this.contextMenuStripWifis.SuspendLayout();
            this.SuspendLayout();
            // 
            // listBoxWifis
            // 
            this.listBoxWifis.FormattingEnabled = true;
            this.listBoxWifis.ItemHeight = 12;
            this.listBoxWifis.Location = new System.Drawing.Point(12, 12);
            this.listBoxWifis.Name = "listBoxWifis";
            this.listBoxWifis.Size = new System.Drawing.Size(460, 436);
            this.listBoxWifis.TabIndex = 0;
            // 
            // contextMenuStripWifis
            // 
            this.contextMenuStripWifis.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemConn,
            this.toolStripMenuItemAdd,
            this.toolStripMenuItemDel});
            this.contextMenuStripWifis.Name = "contextMenuStripWifis";
            this.contextMenuStripWifis.Size = new System.Drawing.Size(181, 92);
            this.contextMenuStripWifis.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenuStripWifis_Opening);
            // 
            // toolStripMenuItemConn
            // 
            this.toolStripMenuItemConn.Name = "toolStripMenuItemConn";
            this.toolStripMenuItemConn.Size = new System.Drawing.Size(180, 22);
            this.toolStripMenuItemConn.Text = "连接";
            this.toolStripMenuItemConn.Click += new System.EventHandler(this.toolStripMenuItemConn_Click);
            // 
            // toolStripMenuItemAdd
            // 
            this.toolStripMenuItemAdd.Name = "toolStripMenuItemAdd";
            this.toolStripMenuItemAdd.Size = new System.Drawing.Size(180, 22);
            this.toolStripMenuItemAdd.Text = "添加";
            this.toolStripMenuItemAdd.Click += new System.EventHandler(this.toolStripMenuItemAdd_Click);
            // 
            // toolStripMenuItemDel
            // 
            this.toolStripMenuItemDel.Name = "toolStripMenuItemDel";
            this.toolStripMenuItemDel.Size = new System.Drawing.Size(180, 22);
            this.toolStripMenuItemDel.Text = "删除";
            this.toolStripMenuItemDel.Click += new System.EventHandler(this.toolStripMenuItemDel_Click);
            // 
            // FormConnWifi
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(484, 461);
            this.ContextMenuStrip = this.contextMenuStripWifis;
            this.Controls.Add(this.listBoxWifis);
            this.Name = "FormConnWifi";
            this.Text = "连接WiFi";
            this.contextMenuStripWifis.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListBox listBoxWifis;
        private System.Windows.Forms.ContextMenuStrip contextMenuStripWifis;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemConn;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemAdd;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemDel;
    }
}