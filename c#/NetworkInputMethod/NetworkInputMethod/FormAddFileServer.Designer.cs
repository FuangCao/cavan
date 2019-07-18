namespace NetworkInputMethod
{
    partial class FormAddFileServer
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
            this.comboBoxProtocl = new System.Windows.Forms.ComboBox();
            this.textBoxUser = new System.Windows.Forms.TextBox();
            this.textBoxPass = new System.Windows.Forms.TextBox();
            this.textBoxCert = new System.Windows.Forms.TextBox();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.buttonOK = new System.Windows.Forms.Button();
            this.labelUser = new System.Windows.Forms.Label();
            this.labelPass = new System.Windows.Forms.Label();
            this.labelCert = new System.Windows.Forms.Label();
            this.labelHost = new System.Windows.Forms.Label();
            this.labelPort = new System.Windows.Forms.Label();
            this.textBoxHost = new System.Windows.Forms.TextBox();
            this.textBoxPort = new System.Windows.Forms.TextBox();
            this.labelProtocol = new System.Windows.Forms.Label();
            this.openFileDialogCert = new System.Windows.Forms.OpenFileDialog();
            this.buttonCert = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // comboBoxProtocl
            // 
            this.comboBoxProtocl.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxProtocl.FormattingEnabled = true;
            this.comboBoxProtocl.Items.AddRange(new object[] {
            "http",
            "https",
            "sftp"});
            this.comboBoxProtocl.Location = new System.Drawing.Point(59, 12);
            this.comboBoxProtocl.Name = "comboBoxProtocl";
            this.comboBoxProtocl.Size = new System.Drawing.Size(213, 20);
            this.comboBoxProtocl.TabIndex = 0;
            // 
            // textBoxUser
            // 
            this.textBoxUser.Location = new System.Drawing.Point(59, 92);
            this.textBoxUser.Name = "textBoxUser";
            this.textBoxUser.Size = new System.Drawing.Size(213, 21);
            this.textBoxUser.TabIndex = 1;
            // 
            // textBoxPass
            // 
            this.textBoxPass.Location = new System.Drawing.Point(59, 121);
            this.textBoxPass.Name = "textBoxPass";
            this.textBoxPass.Size = new System.Drawing.Size(213, 21);
            this.textBoxPass.TabIndex = 2;
            // 
            // textBoxCert
            // 
            this.textBoxCert.Location = new System.Drawing.Point(59, 148);
            this.textBoxCert.Name = "textBoxCert";
            this.textBoxCert.Size = new System.Drawing.Size(174, 21);
            this.textBoxCert.TabIndex = 3;
            // 
            // buttonCancel
            // 
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(14, 192);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 4;
            this.buttonCancel.Text = "取消";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // buttonOK
            // 
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Location = new System.Drawing.Point(197, 192);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(75, 23);
            this.buttonOK.TabIndex = 5;
            this.buttonOK.Text = "确定";
            this.buttonOK.UseVisualStyleBackColor = true;
            // 
            // labelUser
            // 
            this.labelUser.AutoSize = true;
            this.labelUser.Location = new System.Drawing.Point(12, 95);
            this.labelUser.Name = "labelUser";
            this.labelUser.Size = new System.Drawing.Size(41, 12);
            this.labelUser.TabIndex = 6;
            this.labelUser.Text = "用户：";
            // 
            // labelPass
            // 
            this.labelPass.AutoSize = true;
            this.labelPass.Location = new System.Drawing.Point(12, 124);
            this.labelPass.Name = "labelPass";
            this.labelPass.Size = new System.Drawing.Size(41, 12);
            this.labelPass.TabIndex = 7;
            this.labelPass.Text = "密码：";
            // 
            // labelCert
            // 
            this.labelCert.AutoSize = true;
            this.labelCert.Location = new System.Drawing.Point(12, 151);
            this.labelCert.Name = "labelCert";
            this.labelCert.Size = new System.Drawing.Size(41, 12);
            this.labelCert.TabIndex = 8;
            this.labelCert.Text = "证书：";
            // 
            // labelHost
            // 
            this.labelHost.AutoSize = true;
            this.labelHost.Location = new System.Drawing.Point(12, 41);
            this.labelHost.Name = "labelHost";
            this.labelHost.Size = new System.Drawing.Size(41, 12);
            this.labelHost.TabIndex = 9;
            this.labelHost.Text = "地址：";
            // 
            // labelPort
            // 
            this.labelPort.AutoSize = true;
            this.labelPort.Location = new System.Drawing.Point(12, 68);
            this.labelPort.Name = "labelPort";
            this.labelPort.Size = new System.Drawing.Size(41, 12);
            this.labelPort.TabIndex = 10;
            this.labelPort.Text = "端口：";
            // 
            // textBoxHost
            // 
            this.textBoxHost.Location = new System.Drawing.Point(59, 38);
            this.textBoxHost.Name = "textBoxHost";
            this.textBoxHost.Size = new System.Drawing.Size(213, 21);
            this.textBoxHost.TabIndex = 11;
            this.textBoxHost.Text = "192.168.88.140";
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(59, 65);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(213, 21);
            this.textBoxPort.TabIndex = 12;
            // 
            // labelProtocol
            // 
            this.labelProtocol.AutoSize = true;
            this.labelProtocol.Location = new System.Drawing.Point(12, 15);
            this.labelProtocol.Name = "labelProtocol";
            this.labelProtocol.Size = new System.Drawing.Size(41, 12);
            this.labelProtocol.TabIndex = 13;
            this.labelProtocol.Text = "协议：";
            // 
            // openFileDialogCert
            // 
            this.openFileDialogCert.Filter = "证书|*.pem|所有文件|*.*";
            // 
            // buttonCert
            // 
            this.buttonCert.AutoSize = true;
            this.buttonCert.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.buttonCert.Location = new System.Drawing.Point(239, 147);
            this.buttonCert.Name = "buttonCert";
            this.buttonCert.Size = new System.Drawing.Size(33, 22);
            this.buttonCert.TabIndex = 14;
            this.buttonCert.Text = "...";
            this.buttonCert.UseVisualStyleBackColor = true;
            this.buttonCert.Click += new System.EventHandler(this.buttonCert_Click);
            // 
            // FormAddFileServer
            // 
            this.AcceptButton = this.buttonOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(284, 227);
            this.Controls.Add(this.buttonCert);
            this.Controls.Add(this.labelProtocol);
            this.Controls.Add(this.textBoxPort);
            this.Controls.Add(this.textBoxHost);
            this.Controls.Add(this.labelPort);
            this.Controls.Add(this.labelHost);
            this.Controls.Add(this.labelCert);
            this.Controls.Add(this.labelPass);
            this.Controls.Add(this.labelUser);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.textBoxCert);
            this.Controls.Add(this.textBoxPass);
            this.Controls.Add(this.textBoxUser);
            this.Controls.Add(this.comboBoxProtocl);
            this.Name = "FormAddFileServer";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "文件服务器";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox comboBoxProtocl;
        private System.Windows.Forms.TextBox textBoxUser;
        private System.Windows.Forms.TextBox textBoxPass;
        private System.Windows.Forms.TextBox textBoxCert;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.Label labelUser;
        private System.Windows.Forms.Label labelPass;
        private System.Windows.Forms.Label labelCert;
        private System.Windows.Forms.Label labelHost;
        private System.Windows.Forms.Label labelPort;
        private System.Windows.Forms.TextBox textBoxHost;
        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Label labelProtocol;
        private System.Windows.Forms.OpenFileDialog openFileDialogCert;
        private System.Windows.Forms.Button buttonCert;
    }
}