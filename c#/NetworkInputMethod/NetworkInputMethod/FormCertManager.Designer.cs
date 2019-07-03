namespace NetworkInputMethod
{
    partial class FormCertManager
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
            this.listViewCerts = new System.Windows.Forms.ListView();
            this.columnHeaderDomain = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderPassword = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderPathname = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.buttonCancel = new System.Windows.Forms.Button();
            this.buttonOK = new System.Windows.Forms.Button();
            this.textBoxDomain = new System.Windows.Forms.TextBox();
            this.textBoxPassword = new System.Windows.Forms.TextBox();
            this.buttonAdd = new System.Windows.Forms.Button();
            this.labelDomain = new System.Windows.Forms.Label();
            this.labelPassword = new System.Windows.Forms.Label();
            this.openFileDialogCert = new System.Windows.Forms.OpenFileDialog();
            this.contextMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.ToolStripMenuItemDelete = new System.Windows.Forms.ToolStripMenuItem();
            this.contextMenuStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // listViewCerts
            // 
            this.listViewCerts.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderDomain,
            this.columnHeaderPassword,
            this.columnHeaderPathname});
            this.listViewCerts.ContextMenuStrip = this.contextMenuStrip;
            this.listViewCerts.FullRowSelect = true;
            this.listViewCerts.Location = new System.Drawing.Point(12, 12);
            this.listViewCerts.Name = "listViewCerts";
            this.listViewCerts.Size = new System.Drawing.Size(613, 431);
            this.listViewCerts.TabIndex = 0;
            this.listViewCerts.UseCompatibleStateImageBehavior = false;
            this.listViewCerts.View = System.Windows.Forms.View.Details;
            // 
            // columnHeaderDomain
            // 
            this.columnHeaderDomain.Text = "域名";
            this.columnHeaderDomain.Width = 103;
            // 
            // columnHeaderPassword
            // 
            this.columnHeaderPassword.Text = "密码";
            this.columnHeaderPassword.Width = 116;
            // 
            // columnHeaderPathname
            // 
            this.columnHeaderPathname.Text = "路径";
            this.columnHeaderPathname.Width = 389;
            // 
            // buttonCancel
            // 
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(12, 513);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 1;
            this.buttonCancel.Text = "取消";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // buttonOK
            // 
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Location = new System.Drawing.Point(550, 513);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(75, 23);
            this.buttonOK.TabIndex = 2;
            this.buttonOK.Text = "确定";
            this.buttonOK.UseVisualStyleBackColor = true;
            this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
            // 
            // textBoxDomain
            // 
            this.textBoxDomain.Location = new System.Drawing.Point(59, 449);
            this.textBoxDomain.Name = "textBoxDomain";
            this.textBoxDomain.Size = new System.Drawing.Size(566, 21);
            this.textBoxDomain.TabIndex = 3;
            // 
            // textBoxPassword
            // 
            this.textBoxPassword.Location = new System.Drawing.Point(59, 476);
            this.textBoxPassword.Name = "textBoxPassword";
            this.textBoxPassword.Size = new System.Drawing.Size(566, 21);
            this.textBoxPassword.TabIndex = 4;
            // 
            // buttonAdd
            // 
            this.buttonAdd.Location = new System.Drawing.Point(272, 513);
            this.buttonAdd.Name = "buttonAdd";
            this.buttonAdd.Size = new System.Drawing.Size(75, 23);
            this.buttonAdd.TabIndex = 5;
            this.buttonAdd.Text = "添加";
            this.buttonAdd.UseVisualStyleBackColor = true;
            this.buttonAdd.Click += new System.EventHandler(this.buttonAdd_Click);
            // 
            // labelDomain
            // 
            this.labelDomain.AutoSize = true;
            this.labelDomain.Location = new System.Drawing.Point(12, 452);
            this.labelDomain.Name = "labelDomain";
            this.labelDomain.Size = new System.Drawing.Size(41, 12);
            this.labelDomain.TabIndex = 6;
            this.labelDomain.Text = "域名：";
            // 
            // labelPassword
            // 
            this.labelPassword.AutoSize = true;
            this.labelPassword.Location = new System.Drawing.Point(10, 479);
            this.labelPassword.Name = "labelPassword";
            this.labelPassword.Size = new System.Drawing.Size(41, 12);
            this.labelPassword.TabIndex = 7;
            this.labelPassword.Text = "密码：";
            // 
            // openFileDialogCert
            // 
            this.openFileDialogCert.Filter = "证书文件|*.pfx|所有文件|*.*";
            // 
            // contextMenuStrip
            // 
            this.contextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ToolStripMenuItemDelete});
            this.contextMenuStrip.Name = "contextMenuStrip";
            this.contextMenuStrip.Size = new System.Drawing.Size(101, 26);
            // 
            // ToolStripMenuItemDelete
            // 
            this.ToolStripMenuItemDelete.Name = "ToolStripMenuItemDelete";
            this.ToolStripMenuItemDelete.Size = new System.Drawing.Size(152, 22);
            this.ToolStripMenuItemDelete.Text = "删除";
            this.ToolStripMenuItemDelete.Click += new System.EventHandler(this.ToolStripMenuItemDelete_Click);
            // 
            // FormCertManager
            // 
            this.AcceptButton = this.buttonOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(637, 548);
            this.Controls.Add(this.labelPassword);
            this.Controls.Add(this.labelDomain);
            this.Controls.Add(this.buttonAdd);
            this.Controls.Add(this.textBoxPassword);
            this.Controls.Add(this.textBoxDomain);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.listViewCerts);
            this.Name = "FormCertManager";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "证书管理器";
            this.contextMenuStrip.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListView listViewCerts;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.ColumnHeader columnHeaderDomain;
        private System.Windows.Forms.ColumnHeader columnHeaderPassword;
        private System.Windows.Forms.ColumnHeader columnHeaderPathname;
        private System.Windows.Forms.TextBox textBoxDomain;
        private System.Windows.Forms.TextBox textBoxPassword;
        private System.Windows.Forms.Button buttonAdd;
        private System.Windows.Forms.Label labelDomain;
        private System.Windows.Forms.Label labelPassword;
        private System.Windows.Forms.OpenFileDialog openFileDialogCert;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem ToolStripMenuItemDelete;
    }
}