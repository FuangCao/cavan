namespace JwaooOtpProgrammer {
    partial class CavanMacAddressManager {
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
            this.labelAddressStart = new System.Windows.Forms.Label();
            this.labelAddressEnd = new System.Windows.Forms.Label();
            this.labelAddressCount = new System.Windows.Forms.Label();
            this.textBoxAddressStart = new System.Windows.Forms.TextBox();
            this.textBoxAddressCount = new System.Windows.Forms.TextBox();
            this.textBoxAddressEnd = new System.Windows.Forms.TextBox();
            this.contextMenuStripAlloc = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItemAlloc = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemFree = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItemCopy = new System.Windows.Forms.ToolStripMenuItem();
            this.panelAddresses = new System.Windows.Forms.Panel();
            this.listViewAddresses = new System.Windows.Forms.ListView();
            this.columnHeaderAddressStart = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderAddressEnd = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderAddressCount = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderState = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.timerEventLock = new System.Windows.Forms.Timer(this.components);
            this.labelAddressNext = new System.Windows.Forms.Label();
            this.textBoxAddressNext = new System.Windows.Forms.TextBox();
            this.buttonAllocAvg = new System.Windows.Forms.Button();
            this.buttonAllocFixSize = new System.Windows.Forms.Button();
            this.buttonClear = new System.Windows.Forms.Button();
            this.buttonExport = new System.Windows.Forms.Button();
            this.saveFileDialogExport = new System.Windows.Forms.SaveFileDialog();
            this.contextMenuStripAlloc.SuspendLayout();
            this.SuspendLayout();
            // 
            // labelAddressStart
            // 
            this.labelAddressStart.AutoSize = true;
            this.labelAddressStart.Location = new System.Drawing.Point(5, 12);
            this.labelAddressStart.Name = "labelAddressStart";
            this.labelAddressStart.Size = new System.Drawing.Size(65, 12);
            this.labelAddressStart.TabIndex = 1;
            this.labelAddressStart.Text = "起始地址：";
            // 
            // labelAddressEnd
            // 
            this.labelAddressEnd.AutoSize = true;
            this.labelAddressEnd.Location = new System.Drawing.Point(484, 12);
            this.labelAddressEnd.Name = "labelAddressEnd";
            this.labelAddressEnd.Size = new System.Drawing.Size(65, 12);
            this.labelAddressEnd.TabIndex = 2;
            this.labelAddressEnd.Text = "结束地址：";
            // 
            // labelAddressCount
            // 
            this.labelAddressCount.AutoSize = true;
            this.labelAddressCount.Location = new System.Drawing.Point(5, 40);
            this.labelAddressCount.Name = "labelAddressCount";
            this.labelAddressCount.Size = new System.Drawing.Size(65, 12);
            this.labelAddressCount.TabIndex = 3;
            this.labelAddressCount.Text = "地址个数：";
            // 
            // textBoxAddressStart
            // 
            this.textBoxAddressStart.CharacterCasing = System.Windows.Forms.CharacterCasing.Upper;
            this.textBoxAddressStart.Location = new System.Drawing.Point(76, 8);
            this.textBoxAddressStart.Name = "textBoxAddressStart";
            this.textBoxAddressStart.Size = new System.Drawing.Size(350, 21);
            this.textBoxAddressStart.TabIndex = 4;
            this.textBoxAddressStart.Text = "00:00:00:00:00:00";
            this.textBoxAddressStart.TextChanged += new System.EventHandler(this.textBoxAddressStart_TextChanged);
            this.textBoxAddressStart.Leave += new System.EventHandler(this.textBoxAddressStart_Leave);
            // 
            // textBoxAddressCount
            // 
            this.textBoxAddressCount.Location = new System.Drawing.Point(76, 36);
            this.textBoxAddressCount.Name = "textBoxAddressCount";
            this.textBoxAddressCount.Size = new System.Drawing.Size(350, 21);
            this.textBoxAddressCount.TabIndex = 5;
            this.textBoxAddressCount.Text = "1000";
            this.textBoxAddressCount.TextChanged += new System.EventHandler(this.textBoxAddressCount_TextChanged);
            this.textBoxAddressCount.Leave += new System.EventHandler(this.textBoxAddressCount_Leave);
            // 
            // textBoxAddressEnd
            // 
            this.textBoxAddressEnd.CharacterCasing = System.Windows.Forms.CharacterCasing.Upper;
            this.textBoxAddressEnd.Location = new System.Drawing.Point(555, 8);
            this.textBoxAddressEnd.Name = "textBoxAddressEnd";
            this.textBoxAddressEnd.Size = new System.Drawing.Size(350, 21);
            this.textBoxAddressEnd.TabIndex = 6;
            this.textBoxAddressEnd.Text = "00:00:00:00:00:00";
            this.textBoxAddressEnd.TextChanged += new System.EventHandler(this.textBoxAddressEnd_TextChanged);
            this.textBoxAddressEnd.Leave += new System.EventHandler(this.textBoxAddressEnd_Leave);
            // 
            // contextMenuStripAlloc
            // 
            this.contextMenuStripAlloc.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemAlloc,
            this.toolStripMenuItemFree,
            this.ToolStripMenuItemCopy});
            this.contextMenuStripAlloc.Name = "contextMenuStripAlloc";
            this.contextMenuStripAlloc.Size = new System.Drawing.Size(137, 70);
            // 
            // toolStripMenuItemAlloc
            // 
            this.toolStripMenuItemAlloc.Name = "toolStripMenuItemAlloc";
            this.toolStripMenuItemAlloc.Size = new System.Drawing.Size(136, 22);
            this.toolStripMenuItemAlloc.Text = "新增地址段";
            this.toolStripMenuItemAlloc.Click += new System.EventHandler(this.toolStripMenuItemAlloc_Click);
            // 
            // toolStripMenuItemFree
            // 
            this.toolStripMenuItemFree.Name = "toolStripMenuItemFree";
            this.toolStripMenuItemFree.Size = new System.Drawing.Size(136, 22);
            this.toolStripMenuItemFree.Text = "删除地址段";
            this.toolStripMenuItemFree.Click += new System.EventHandler(this.toolStripMenuItemFree_Click);
            // 
            // ToolStripMenuItemCopy
            // 
            this.ToolStripMenuItemCopy.Name = "ToolStripMenuItemCopy";
            this.ToolStripMenuItemCopy.Size = new System.Drawing.Size(136, 22);
            this.ToolStripMenuItemCopy.Text = "复制";
            this.ToolStripMenuItemCopy.Click += new System.EventHandler(this.ToolStripMenuItemCopy_Click);
            // 
            // panelAddresses
            // 
            this.panelAddresses.Location = new System.Drawing.Point(5, 64);
            this.panelAddresses.Name = "panelAddresses";
            this.panelAddresses.Size = new System.Drawing.Size(900, 89);
            this.panelAddresses.TabIndex = 9;
            // 
            // listViewAddresses
            // 
            this.listViewAddresses.Alignment = System.Windows.Forms.ListViewAlignment.Left;
            this.listViewAddresses.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderAddressStart,
            this.columnHeaderAddressEnd,
            this.columnHeaderAddressCount,
            this.columnHeaderState});
            this.listViewAddresses.FullRowSelect = true;
            this.listViewAddresses.GridLines = true;
            this.listViewAddresses.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.listViewAddresses.Location = new System.Drawing.Point(5, 160);
            this.listViewAddresses.MultiSelect = false;
            this.listViewAddresses.Name = "listViewAddresses";
            this.listViewAddresses.Size = new System.Drawing.Size(900, 200);
            this.listViewAddresses.TabIndex = 10;
            this.listViewAddresses.UseCompatibleStateImageBehavior = false;
            this.listViewAddresses.View = System.Windows.Forms.View.Details;
            this.listViewAddresses.SelectedIndexChanged += new System.EventHandler(this.listViewAddresses_SelectedIndexChanged);
            this.listViewAddresses.Enter += new System.EventHandler(this.listViewAddresses_SelectedIndexChanged);
            this.listViewAddresses.Leave += new System.EventHandler(this.listViewAddresses_Leave);
            // 
            // columnHeaderAddressStart
            // 
            this.columnHeaderAddressStart.Text = "起始地址";
            this.columnHeaderAddressStart.Width = 300;
            // 
            // columnHeaderAddressEnd
            // 
            this.columnHeaderAddressEnd.Text = "结束地址";
            this.columnHeaderAddressEnd.Width = 300;
            // 
            // columnHeaderAddressCount
            // 
            this.columnHeaderAddressCount.Text = "地址个数";
            this.columnHeaderAddressCount.Width = 200;
            // 
            // columnHeaderState
            // 
            this.columnHeaderState.Text = "状态";
            this.columnHeaderState.Width = 96;
            // 
            // timerEventLock
            // 
            this.timerEventLock.Interval = 200;
            this.timerEventLock.Tick += new System.EventHandler(this.timerUpdateLock_Tick);
            // 
            // labelAddressNext
            // 
            this.labelAddressNext.AutoSize = true;
            this.labelAddressNext.Location = new System.Drawing.Point(448, 40);
            this.labelAddressNext.Name = "labelAddressNext";
            this.labelAddressNext.Size = new System.Drawing.Size(101, 12);
            this.labelAddressNext.TabIndex = 11;
            this.labelAddressNext.Text = "下一次起始地址：";
            // 
            // textBoxAddressNext
            // 
            this.textBoxAddressNext.CharacterCasing = System.Windows.Forms.CharacterCasing.Upper;
            this.textBoxAddressNext.Location = new System.Drawing.Point(555, 36);
            this.textBoxAddressNext.Name = "textBoxAddressNext";
            this.textBoxAddressNext.Size = new System.Drawing.Size(350, 21);
            this.textBoxAddressNext.TabIndex = 12;
            this.textBoxAddressNext.Text = "00:00:00:00:00:00";
            this.textBoxAddressNext.TextChanged += new System.EventHandler(this.textBoxAddressNext_TextChanged);
            this.textBoxAddressNext.Leave += new System.EventHandler(this.textBoxAddressNext_Leave);
            // 
            // buttonAllocAvg
            // 
            this.buttonAllocAvg.Location = new System.Drawing.Point(5, 367);
            this.buttonAllocAvg.Name = "buttonAllocAvg";
            this.buttonAllocAvg.Size = new System.Drawing.Size(95, 49);
            this.buttonAllocAvg.TabIndex = 13;
            this.buttonAllocAvg.Text = "平均分配";
            this.buttonAllocAvg.UseVisualStyleBackColor = true;
            this.buttonAllocAvg.Click += new System.EventHandler(this.buttonAllocAvg_Click);
            // 
            // buttonAllocFixSize
            // 
            this.buttonAllocFixSize.Location = new System.Drawing.Point(273, 367);
            this.buttonAllocFixSize.Name = "buttonAllocFixSize";
            this.buttonAllocFixSize.Size = new System.Drawing.Size(95, 49);
            this.buttonAllocFixSize.TabIndex = 14;
            this.buttonAllocFixSize.Text = "固定大小分配";
            this.buttonAllocFixSize.UseVisualStyleBackColor = true;
            this.buttonAllocFixSize.Click += new System.EventHandler(this.buttonAllocFixSize_Click);
            // 
            // buttonClear
            // 
            this.buttonClear.Location = new System.Drawing.Point(541, 367);
            this.buttonClear.Name = "buttonClear";
            this.buttonClear.Size = new System.Drawing.Size(95, 49);
            this.buttonClear.TabIndex = 15;
            this.buttonClear.Text = "重新分配";
            this.buttonClear.UseVisualStyleBackColor = true;
            this.buttonClear.Click += new System.EventHandler(this.buttonClear_Click);
            // 
            // buttonExport
            // 
            this.buttonExport.Location = new System.Drawing.Point(809, 367);
            this.buttonExport.Name = "buttonExport";
            this.buttonExport.Size = new System.Drawing.Size(95, 49);
            this.buttonExport.TabIndex = 16;
            this.buttonExport.Text = "导出";
            this.buttonExport.UseVisualStyleBackColor = true;
            this.buttonExport.Click += new System.EventHandler(this.buttonExport_Click);
            // 
            // saveFileDialogExport
            // 
            this.saveFileDialogExport.DefaultExt = "txt";
            this.saveFileDialogExport.FileName = "MacAddressMap";
            this.saveFileDialogExport.Filter = "文本文件|*.txt";
            this.saveFileDialogExport.Title = "导出MAC地址列表";
            // 
            // CavanMacAddressManager
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoScroll = true;
            this.AutoSize = true;
            this.ClientSize = new System.Drawing.Size(911, 423);
            this.Controls.Add(this.buttonExport);
            this.Controls.Add(this.buttonClear);
            this.Controls.Add(this.buttonAllocFixSize);
            this.Controls.Add(this.buttonAllocAvg);
            this.Controls.Add(this.textBoxAddressNext);
            this.Controls.Add(this.labelAddressNext);
            this.Controls.Add(this.listViewAddresses);
            this.Controls.Add(this.panelAddresses);
            this.Controls.Add(this.textBoxAddressEnd);
            this.Controls.Add(this.textBoxAddressCount);
            this.Controls.Add(this.textBoxAddressStart);
            this.Controls.Add(this.labelAddressCount);
            this.Controls.Add(this.labelAddressEnd);
            this.Controls.Add(this.labelAddressStart);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Name = "CavanMacAddressManager";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "井蛙MAC地址划分";
            this.contextMenuStripAlloc.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Label labelAddressStart;
        private System.Windows.Forms.Label labelAddressEnd;
        private System.Windows.Forms.Label labelAddressCount;
        private System.Windows.Forms.TextBox textBoxAddressStart;
        private System.Windows.Forms.TextBox textBoxAddressCount;
        private System.Windows.Forms.TextBox textBoxAddressEnd;
        private System.Windows.Forms.ContextMenuStrip contextMenuStripAlloc;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemAlloc;
        private System.Windows.Forms.Panel panelAddresses;
        private System.Windows.Forms.ListView listViewAddresses;
        private System.Windows.Forms.ColumnHeader columnHeaderAddressStart;
        private System.Windows.Forms.ColumnHeader columnHeaderAddressEnd;
        private System.Windows.Forms.ColumnHeader columnHeaderAddressCount;
        private System.Windows.Forms.ColumnHeader columnHeaderState;
        private System.Windows.Forms.Timer timerEventLock;
        private System.Windows.Forms.Label labelAddressNext;
        private System.Windows.Forms.TextBox textBoxAddressNext;
        private System.Windows.Forms.Button buttonAllocAvg;
        private System.Windows.Forms.Button buttonAllocFixSize;
        private System.Windows.Forms.Button buttonClear;
        private System.Windows.Forms.Button buttonExport;
        private System.Windows.Forms.SaveFileDialog saveFileDialogExport;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItemCopy;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemFree;
    }
}