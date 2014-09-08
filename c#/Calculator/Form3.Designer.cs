namespace CFA090307A
{
    partial class Form3
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
        /// 设计器支持所需的方法 - 不要
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form3));
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
            this.listView1 = new System.Windows.Forms.ListView();
            this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItem_GetFormula = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem_GetResult = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_DeleteAll = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_Hide = new System.Windows.Forms.ToolStripMenuItem();
            this.ToolStripMenuItem_Close = new System.Windows.Forms.ToolStripMenuItem();
            this.button_Getformula = new System.Windows.Forms.Button();
            this.button_GetResult = new System.Windows.Forms.Button();
            this.button_Close = new System.Windows.Forms.Button();
            this.button_Hide = new System.Windows.Forms.Button();
            this.button_DeleteAll = new System.Windows.Forms.Button();
            this.contextMenuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "计算时间";
            this.columnHeader1.Width = 157;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "算式";
            this.columnHeader2.Width = 180;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "结果";
            this.columnHeader3.Width = 120;
            // 
            // listView1
            // 
            this.listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader3});
            this.listView1.ContextMenuStrip = this.contextMenuStrip1;
            this.listView1.Dock = System.Windows.Forms.DockStyle.Top;
            this.listView1.FullRowSelect = true;
            this.listView1.GridLines = true;
            this.listView1.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.listView1.Location = new System.Drawing.Point(0, 0);
            this.listView1.MultiSelect = false;
            this.listView1.Name = "listView1";
            this.listView1.Size = new System.Drawing.Size(461, 432);
            this.listView1.TabIndex = 0;
            this.listView1.UseCompatibleStateImageBehavior = false;
            this.listView1.View = System.Windows.Forms.View.Details;
            this.listView1.SelectedIndexChanged += new System.EventHandler(this.listView1_SelectedIndexChanged);
            // 
            // contextMenuStrip1
            // 
            this.contextMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItem_GetFormula,
            this.toolStripMenuItem_GetResult,
            this.ToolStripMenuItem_DeleteAll,
            this.ToolStripMenuItem_Hide,
            this.ToolStripMenuItem_Close});
            this.contextMenuStrip1.Name = "contextMenuStrip1";
            this.contextMenuStrip1.Size = new System.Drawing.Size(123, 114);
            // 
            // toolStripMenuItem_GetFormula
            // 
            this.toolStripMenuItem_GetFormula.Name = "toolStripMenuItem_GetFormula";
            this.toolStripMenuItem_GetFormula.Size = new System.Drawing.Size(122, 22);
            this.toolStripMenuItem_GetFormula.Text = "获取算式";
            // 
            // toolStripMenuItem_GetResult
            // 
            this.toolStripMenuItem_GetResult.Name = "toolStripMenuItem_GetResult";
            this.toolStripMenuItem_GetResult.Size = new System.Drawing.Size(122, 22);
            this.toolStripMenuItem_GetResult.Text = "获取结果";
            // 
            // ToolStripMenuItem_DeleteAll
            // 
            this.ToolStripMenuItem_DeleteAll.Name = "ToolStripMenuItem_DeleteAll";
            this.ToolStripMenuItem_DeleteAll.Size = new System.Drawing.Size(122, 22);
            this.ToolStripMenuItem_DeleteAll.Text = "清空日志";
            this.ToolStripMenuItem_DeleteAll.Click += new System.EventHandler(this.button_DeleteAll_Click);
            // 
            // ToolStripMenuItem_Hide
            // 
            this.ToolStripMenuItem_Hide.Name = "ToolStripMenuItem_Hide";
            this.ToolStripMenuItem_Hide.Size = new System.Drawing.Size(122, 22);
            this.ToolStripMenuItem_Hide.Text = "隐藏窗口";
            // 
            // ToolStripMenuItem_Close
            // 
            this.ToolStripMenuItem_Close.Name = "ToolStripMenuItem_Close";
            this.ToolStripMenuItem_Close.Size = new System.Drawing.Size(122, 22);
            this.ToolStripMenuItem_Close.Text = "关闭";
            this.ToolStripMenuItem_Close.Click += new System.EventHandler(this.button_Close_Click);
            // 
            // button_Getformula
            // 
            this.button_Getformula.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.button_Getformula.Location = new System.Drawing.Point(13, 438);
            this.button_Getformula.Name = "button_Getformula";
            this.button_Getformula.Size = new System.Drawing.Size(75, 36);
            this.button_Getformula.TabIndex = 1;
            this.button_Getformula.Text = "获取算式";
            this.button_Getformula.UseVisualStyleBackColor = true;
            // 
            // button_GetResult
            // 
            this.button_GetResult.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.button_GetResult.Location = new System.Drawing.Point(103, 438);
            this.button_GetResult.Name = "button_GetResult";
            this.button_GetResult.Size = new System.Drawing.Size(75, 36);
            this.button_GetResult.TabIndex = 2;
            this.button_GetResult.Text = "获取结果";
            this.button_GetResult.UseVisualStyleBackColor = true;
            // 
            // button_Close
            // 
            this.button_Close.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.button_Close.Location = new System.Drawing.Point(283, 438);
            this.button_Close.Name = "button_Close";
            this.button_Close.Size = new System.Drawing.Size(75, 36);
            this.button_Close.TabIndex = 3;
            this.button_Close.Text = "关  闭";
            this.button_Close.UseVisualStyleBackColor = true;
            this.button_Close.Click += new System.EventHandler(this.button_Close_Click);
            // 
            // button_Hide
            // 
            this.button_Hide.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.button_Hide.Location = new System.Drawing.Point(373, 438);
            this.button_Hide.Name = "button_Hide";
            this.button_Hide.Size = new System.Drawing.Size(75, 36);
            this.button_Hide.TabIndex = 4;
            this.button_Hide.Text = "隐  藏";
            this.button_Hide.UseVisualStyleBackColor = true;
            // 
            // button_DeleteAll
            // 
            this.button_DeleteAll.Font = new System.Drawing.Font("宋体", 10.5F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.button_DeleteAll.Location = new System.Drawing.Point(193, 438);
            this.button_DeleteAll.Name = "button_DeleteAll";
            this.button_DeleteAll.Size = new System.Drawing.Size(75, 36);
            this.button_DeleteAll.TabIndex = 5;
            this.button_DeleteAll.Text = "清空日志";
            this.button_DeleteAll.UseVisualStyleBackColor = true;
            this.button_DeleteAll.Click += new System.EventHandler(this.button_DeleteAll_Click);
            // 
            // Form3
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(192)))), ((int)(((byte)(255)))));
            this.ClientSize = new System.Drawing.Size(461, 481);
            this.Controls.Add(this.button_DeleteAll);
            this.Controls.Add(this.button_Hide);
            this.Controls.Add(this.button_Close);
            this.Controls.Add(this.button_GetResult);
            this.Controls.Add(this.button_Getformula);
            this.Controls.Add(this.listView1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.Name = "Form3";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "计算器日志  计Y062班 曹福昂";
            this.Load += new System.EventHandler(this.Form3_Load);
            this.contextMenuStrip1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        public System.Windows.Forms.ListView listView1;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        public System.Windows.Forms.Button button_Getformula;
        public System.Windows.Forms.Button button_GetResult;
        public System.Windows.Forms.Button button_Close;
        public System.Windows.Forms.Button button_Hide;
        private System.Windows.Forms.Button button_DeleteAll;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_DeleteAll;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_Close;
        public System.Windows.Forms.ContextMenuStrip contextMenuStrip1;
        public System.Windows.Forms.ToolStripMenuItem toolStripMenuItem_GetFormula;
        public System.Windows.Forms.ToolStripMenuItem toolStripMenuItem_GetResult;
        public System.Windows.Forms.ToolStripMenuItem ToolStripMenuItem_Hide;
    }
}