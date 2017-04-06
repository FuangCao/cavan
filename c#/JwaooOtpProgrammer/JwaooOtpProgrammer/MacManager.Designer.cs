namespace JwaooOtpProgrammer {
    partial class MacManager {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing) {
            if (disposing && (components != null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent() {
            this.components = new System.ComponentModel.Container();
            this.listBoxAddresses = new System.Windows.Forms.ListBox();
            this.labelAddressStart = new System.Windows.Forms.Label();
            this.labelAddressEnd = new System.Windows.Forms.Label();
            this.labelAddressCount = new System.Windows.Forms.Label();
            this.textBoxAddressStart = new System.Windows.Forms.TextBox();
            this.textBoxAddressCount = new System.Windows.Forms.TextBox();
            this.textBoxAddressEnd = new System.Windows.Forms.TextBox();
            this.contextMenuStripAlloc = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItemAlloc = new System.Windows.Forms.ToolStripMenuItem();
            this.contextMenuStripFree = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItemFree = new System.Windows.Forms.ToolStripMenuItem();
            this.panelAddresses = new System.Windows.Forms.Panel();
            this.contextMenuStripAlloc.SuspendLayout();
            this.contextMenuStripFree.SuspendLayout();
            this.SuspendLayout();
            // 
            // listBoxAddresses
            // 
            this.listBoxAddresses.FormattingEnabled = true;
            this.listBoxAddresses.ItemHeight = 12;
            this.listBoxAddresses.Location = new System.Drawing.Point(5, 220);
            this.listBoxAddresses.Name = "listBoxAddresses";
            this.listBoxAddresses.Size = new System.Drawing.Size(831, 292);
            this.listBoxAddresses.TabIndex = 0;
            // 
            // labelAddressStart
            // 
            this.labelAddressStart.AutoSize = true;
            this.labelAddressStart.Location = new System.Drawing.Point(5, 10);
            this.labelAddressStart.Name = "labelAddressStart";
            this.labelAddressStart.Size = new System.Drawing.Size(65, 12);
            this.labelAddressStart.TabIndex = 1;
            this.labelAddressStart.Text = "起始地址：";
            // 
            // labelAddressEnd
            // 
            this.labelAddressEnd.AutoSize = true;
            this.labelAddressEnd.Location = new System.Drawing.Point(5, 62);
            this.labelAddressEnd.Name = "labelAddressEnd";
            this.labelAddressEnd.Size = new System.Drawing.Size(65, 12);
            this.labelAddressEnd.TabIndex = 2;
            this.labelAddressEnd.Text = "结束地址：";
            // 
            // labelAddressCount
            // 
            this.labelAddressCount.AutoSize = true;
            this.labelAddressCount.Location = new System.Drawing.Point(5, 36);
            this.labelAddressCount.Name = "labelAddressCount";
            this.labelAddressCount.Size = new System.Drawing.Size(65, 12);
            this.labelAddressCount.TabIndex = 3;
            this.labelAddressCount.Text = "地址个数：";
            // 
            // textBoxAddressStart
            // 
            this.textBoxAddressStart.Location = new System.Drawing.Point(76, 6);
            this.textBoxAddressStart.Name = "textBoxAddressStart";
            this.textBoxAddressStart.Size = new System.Drawing.Size(760, 21);
            this.textBoxAddressStart.TabIndex = 4;
            this.textBoxAddressStart.TextChanged += new System.EventHandler(this.textBoxAddressStart_TextChanged);
            // 
            // textBoxAddressCount
            // 
            this.textBoxAddressCount.Location = new System.Drawing.Point(76, 32);
            this.textBoxAddressCount.Name = "textBoxAddressCount";
            this.textBoxAddressCount.Size = new System.Drawing.Size(760, 21);
            this.textBoxAddressCount.TabIndex = 5;
            this.textBoxAddressCount.TextChanged += new System.EventHandler(this.textBoxAddressCount_TextChanged);
            // 
            // textBoxAddressEnd
            // 
            this.textBoxAddressEnd.Location = new System.Drawing.Point(76, 58);
            this.textBoxAddressEnd.Name = "textBoxAddressEnd";
            this.textBoxAddressEnd.Size = new System.Drawing.Size(760, 21);
            this.textBoxAddressEnd.TabIndex = 6;
            this.textBoxAddressEnd.TextChanged += new System.EventHandler(this.textBoxAddressEnd_TextChanged);
            // 
            // contextMenuStripAlloc
            // 
            this.contextMenuStripAlloc.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemAlloc});
            this.contextMenuStripAlloc.Name = "contextMenuStripAlloc";
            this.contextMenuStripAlloc.Size = new System.Drawing.Size(137, 26);
            // 
            // toolStripMenuItemAlloc
            // 
            this.toolStripMenuItemAlloc.Name = "toolStripMenuItemAlloc";
            this.toolStripMenuItemAlloc.Size = new System.Drawing.Size(136, 22);
            this.toolStripMenuItemAlloc.Text = "增加地址段";
            this.toolStripMenuItemAlloc.Click += new System.EventHandler(this.toolStripMenuItemAlloc_Click);
            // 
            // contextMenuStripFree
            // 
            this.contextMenuStripFree.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemFree});
            this.contextMenuStripFree.Name = "contextMenuStripFree";
            this.contextMenuStripFree.Size = new System.Drawing.Size(137, 26);
            // 
            // toolStripMenuItemFree
            // 
            this.toolStripMenuItemFree.Name = "toolStripMenuItemFree";
            this.toolStripMenuItemFree.Size = new System.Drawing.Size(136, 22);
            this.toolStripMenuItemFree.Text = "删除地址段";
            this.toolStripMenuItemFree.Click += new System.EventHandler(this.toolStripMenuItemFree_Click);
            // 
            // panelAddresses
            // 
            this.panelAddresses.Location = new System.Drawing.Point(5, 95);
            this.panelAddresses.Name = "panelAddresses";
            this.panelAddresses.Size = new System.Drawing.Size(831, 119);
            this.panelAddresses.TabIndex = 9;
            // 
            // MacManager
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoScroll = true;
            this.AutoSize = true;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.ClientSize = new System.Drawing.Size(841, 516);
            this.Controls.Add(this.panelAddresses);
            this.Controls.Add(this.textBoxAddressEnd);
            this.Controls.Add(this.textBoxAddressCount);
            this.Controls.Add(this.textBoxAddressStart);
            this.Controls.Add(this.labelAddressCount);
            this.Controls.Add(this.labelAddressEnd);
            this.Controls.Add(this.labelAddressStart);
            this.Controls.Add(this.listBoxAddresses);
            this.MaximizeBox = false;
            this.Name = "MacManager";
            this.Text = "井蛙地址管理器";
            this.contextMenuStripAlloc.ResumeLayout(false);
            this.contextMenuStripFree.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox listBoxAddresses;
        private System.Windows.Forms.Label labelAddressStart;
        private System.Windows.Forms.Label labelAddressEnd;
        private System.Windows.Forms.Label labelAddressCount;
        private System.Windows.Forms.TextBox textBoxAddressStart;
        private System.Windows.Forms.TextBox textBoxAddressCount;
        private System.Windows.Forms.TextBox textBoxAddressEnd;
        private System.Windows.Forms.ContextMenuStrip contextMenuStripAlloc;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemAlloc;
        private System.Windows.Forms.ContextMenuStrip contextMenuStripFree;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemFree;
        private System.Windows.Forms.Panel panelAddresses;
    }
}