namespace NetworkInputMethod
{
    partial class FormDaemonManager
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
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.listViewCommands = new System.Windows.Forms.ListView();
            this.columnHeaderState = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderCommand = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderArgs = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.contextMenuStripCommands = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItemStart = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemStop = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemRestart = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemAdd = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemEdit = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemRemove = new System.Windows.Forms.ToolStripMenuItem();
            this.panel1 = new System.Windows.Forms.Panel();
            this.checkBoxSelAll = new System.Windows.Forms.CheckBox();
            this.buttonStartAll = new System.Windows.Forms.Button();
            this.buttonRestartAll = new System.Windows.Forms.Button();
            this.buttonStopAll = new System.Windows.Forms.Button();
            this.buttonRestart = new System.Windows.Forms.Button();
            this.buttonStop = new System.Windows.Forms.Button();
            this.buttonStart = new System.Windows.Forms.Button();
            this.checkBoxUseShell = new System.Windows.Forms.CheckBox();
            this.tableLayoutPanel1.SuspendLayout();
            this.contextMenuStripCommands.SuspendLayout();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 1;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Controls.Add(this.listViewCommands, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.panel1, 0, 1);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 0);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 2;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
            this.tableLayoutPanel1.Size = new System.Drawing.Size(784, 561);
            this.tableLayoutPanel1.TabIndex = 0;
            // 
            // listViewCommands
            // 
            this.listViewCommands.CheckBoxes = true;
            this.listViewCommands.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderState,
            this.columnHeaderCommand,
            this.columnHeaderArgs});
            this.listViewCommands.ContextMenuStrip = this.contextMenuStripCommands;
            this.listViewCommands.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listViewCommands.FullRowSelect = true;
            this.listViewCommands.GridLines = true;
            this.listViewCommands.HideSelection = false;
            this.listViewCommands.Location = new System.Drawing.Point(3, 3);
            this.listViewCommands.Name = "listViewCommands";
            this.listViewCommands.Size = new System.Drawing.Size(778, 518);
            this.listViewCommands.TabIndex = 0;
            this.listViewCommands.UseCompatibleStateImageBehavior = false;
            this.listViewCommands.View = System.Windows.Forms.View.Details;
            // 
            // columnHeaderState
            // 
            this.columnHeaderState.Text = "状态";
            this.columnHeaderState.Width = 80;
            // 
            // columnHeaderCommand
            // 
            this.columnHeaderCommand.Text = "命令";
            this.columnHeaderCommand.Width = 392;
            // 
            // columnHeaderArgs
            // 
            this.columnHeaderArgs.Text = "参数";
            this.columnHeaderArgs.Width = 300;
            // 
            // contextMenuStripCommands
            // 
            this.contextMenuStripCommands.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemStart,
            this.toolStripMenuItemStop,
            this.toolStripMenuItemRestart,
            this.toolStripMenuItemAdd,
            this.toolStripMenuItemEdit,
            this.toolStripMenuItemRemove});
            this.contextMenuStripCommands.Name = "contextMenuStripCommands";
            this.contextMenuStripCommands.Size = new System.Drawing.Size(101, 136);
            this.contextMenuStripCommands.Opening += new System.ComponentModel.CancelEventHandler(this.contextMenuStripCommands_Opening);
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
            // toolStripMenuItemRestart
            // 
            this.toolStripMenuItemRestart.Name = "toolStripMenuItemRestart";
            this.toolStripMenuItemRestart.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemRestart.Text = "重启";
            this.toolStripMenuItemRestart.Click += new System.EventHandler(this.toolStripMenuItemRestart_Click);
            // 
            // toolStripMenuItemAdd
            // 
            this.toolStripMenuItemAdd.Name = "toolStripMenuItemAdd";
            this.toolStripMenuItemAdd.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemAdd.Text = "添加";
            this.toolStripMenuItemAdd.Click += new System.EventHandler(this.toolStripMenuItemAdd_Click);
            // 
            // toolStripMenuItemEdit
            // 
            this.toolStripMenuItemEdit.Name = "toolStripMenuItemEdit";
            this.toolStripMenuItemEdit.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemEdit.Text = "编辑";
            this.toolStripMenuItemEdit.Click += new System.EventHandler(this.toolStripMenuItemEdit_Click);
            // 
            // toolStripMenuItemRemove
            // 
            this.toolStripMenuItemRemove.Name = "toolStripMenuItemRemove";
            this.toolStripMenuItemRemove.Size = new System.Drawing.Size(100, 22);
            this.toolStripMenuItemRemove.Text = "删除";
            this.toolStripMenuItemRemove.Click += new System.EventHandler(this.toolStripMenuItemRemove_Click);
            // 
            // panel1
            // 
            this.panel1.AutoSize = true;
            this.panel1.Controls.Add(this.checkBoxSelAll);
            this.panel1.Controls.Add(this.buttonStartAll);
            this.panel1.Controls.Add(this.buttonRestartAll);
            this.panel1.Controls.Add(this.buttonStopAll);
            this.panel1.Controls.Add(this.buttonRestart);
            this.panel1.Controls.Add(this.buttonStop);
            this.panel1.Controls.Add(this.buttonStart);
            this.panel1.Controls.Add(this.checkBoxUseShell);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(3, 527);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(778, 31);
            this.panel1.TabIndex = 1;
            // 
            // checkBoxSelAll
            // 
            this.checkBoxSelAll.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.checkBoxSelAll.AutoSize = true;
            this.checkBoxSelAll.Location = new System.Drawing.Point(9, 8);
            this.checkBoxSelAll.Name = "checkBoxSelAll";
            this.checkBoxSelAll.Size = new System.Drawing.Size(48, 16);
            this.checkBoxSelAll.TabIndex = 7;
            this.checkBoxSelAll.Text = "全选";
            this.checkBoxSelAll.UseVisualStyleBackColor = true;
            this.checkBoxSelAll.CheckedChanged += new System.EventHandler(this.checkBoxSelAll_CheckedChanged);
            // 
            // buttonStartAll
            // 
            this.buttonStartAll.Anchor = System.Windows.Forms.AnchorStyles.Right;
            this.buttonStartAll.Location = new System.Drawing.Point(451, 5);
            this.buttonStartAll.Name = "buttonStartAll";
            this.buttonStartAll.Size = new System.Drawing.Size(75, 23);
            this.buttonStartAll.TabIndex = 6;
            this.buttonStartAll.Text = "全部启动";
            this.buttonStartAll.UseVisualStyleBackColor = true;
            this.buttonStartAll.Click += new System.EventHandler(this.buttonStartAll_Click);
            // 
            // buttonRestartAll
            // 
            this.buttonRestartAll.Anchor = System.Windows.Forms.AnchorStyles.Right;
            this.buttonRestartAll.Location = new System.Drawing.Point(370, 5);
            this.buttonRestartAll.Name = "buttonRestartAll";
            this.buttonRestartAll.Size = new System.Drawing.Size(75, 23);
            this.buttonRestartAll.TabIndex = 5;
            this.buttonRestartAll.Text = "全部重启";
            this.buttonRestartAll.UseVisualStyleBackColor = true;
            this.buttonRestartAll.Click += new System.EventHandler(this.buttonRestartAll_Click);
            // 
            // buttonStopAll
            // 
            this.buttonStopAll.Anchor = System.Windows.Forms.AnchorStyles.Right;
            this.buttonStopAll.Location = new System.Drawing.Point(289, 4);
            this.buttonStopAll.Name = "buttonStopAll";
            this.buttonStopAll.Size = new System.Drawing.Size(75, 23);
            this.buttonStopAll.TabIndex = 4;
            this.buttonStopAll.Text = "全部停止";
            this.buttonStopAll.UseVisualStyleBackColor = true;
            this.buttonStopAll.Click += new System.EventHandler(this.buttonStopAll_Click);
            // 
            // buttonRestart
            // 
            this.buttonRestart.Anchor = System.Windows.Forms.AnchorStyles.Right;
            this.buttonRestart.Location = new System.Drawing.Point(613, 4);
            this.buttonRestart.Name = "buttonRestart";
            this.buttonRestart.Size = new System.Drawing.Size(75, 23);
            this.buttonRestart.TabIndex = 3;
            this.buttonRestart.Text = "重启";
            this.buttonRestart.UseVisualStyleBackColor = true;
            this.buttonRestart.Click += new System.EventHandler(this.buttonRestart_Click);
            // 
            // buttonStop
            // 
            this.buttonStop.Anchor = System.Windows.Forms.AnchorStyles.Right;
            this.buttonStop.Location = new System.Drawing.Point(532, 4);
            this.buttonStop.Name = "buttonStop";
            this.buttonStop.Size = new System.Drawing.Size(75, 23);
            this.buttonStop.TabIndex = 2;
            this.buttonStop.Text = "停止";
            this.buttonStop.UseVisualStyleBackColor = true;
            this.buttonStop.Click += new System.EventHandler(this.buttonStop_Click);
            // 
            // buttonStart
            // 
            this.buttonStart.Anchor = System.Windows.Forms.AnchorStyles.Right;
            this.buttonStart.Location = new System.Drawing.Point(694, 4);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(75, 23);
            this.buttonStart.TabIndex = 1;
            this.buttonStart.Text = "启动";
            this.buttonStart.UseVisualStyleBackColor = true;
            this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
            // 
            // checkBoxUseShell
            // 
            this.checkBoxUseShell.Anchor = System.Windows.Forms.AnchorStyles.Left;
            this.checkBoxUseShell.AutoSize = true;
            this.checkBoxUseShell.Location = new System.Drawing.Point(63, 8);
            this.checkBoxUseShell.Name = "checkBoxUseShell";
            this.checkBoxUseShell.Size = new System.Drawing.Size(96, 16);
            this.checkBoxUseShell.TabIndex = 0;
            this.checkBoxUseShell.Text = "显示命令窗口";
            this.checkBoxUseShell.UseVisualStyleBackColor = true;
            // 
            // FormDaemonManager
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(784, 561);
            this.Controls.Add(this.tableLayoutPanel1);
            this.Name = "FormDaemonManager";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "后台管理器";
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            this.contextMenuStripCommands.ResumeLayout(false);
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.ListView listViewCommands;
        private System.Windows.Forms.ColumnHeader columnHeaderState;
        private System.Windows.Forms.ColumnHeader columnHeaderCommand;
        private System.Windows.Forms.ColumnHeader columnHeaderArgs;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button buttonStop;
        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.CheckBox checkBoxUseShell;
        private System.Windows.Forms.ContextMenuStrip contextMenuStripCommands;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemStart;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemStop;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemEdit;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemRemove;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemAdd;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemRestart;
        private System.Windows.Forms.Button buttonRestart;
        private System.Windows.Forms.Button buttonStopAll;
        private System.Windows.Forms.Button buttonStartAll;
        private System.Windows.Forms.Button buttonRestartAll;
        private System.Windows.Forms.CheckBox checkBoxSelAll;
    }
}