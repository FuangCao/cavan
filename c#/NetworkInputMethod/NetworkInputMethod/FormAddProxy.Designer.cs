﻿namespace NetworkInputMethod
{
    partial class FormAddProxy
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
            this.textBoxServer = new System.Windows.Forms.TextBox();
            this.textBoxPort = new System.Windows.Forms.TextBox();
            this.labelPort = new System.Windows.Forms.Label();
            this.labelServer = new System.Windows.Forms.Label();
            this.labelClientAddress = new System.Windows.Forms.Label();
            this.labelTarget = new System.Windows.Forms.Label();
            this.textBoxClientAddress = new System.Windows.Forms.TextBox();
            this.textBoxTarget = new System.Windows.Forms.TextBox();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.buttonOK = new System.Windows.Forms.Button();
            this.labelClientName = new System.Windows.Forms.Label();
            this.textBoxClientName = new System.Windows.Forms.TextBox();
            this.listBoxClients = new System.Windows.Forms.ListBox();
            this.buttonRefresh = new System.Windows.Forms.Button();
            this.backgroundWorkerRefresh = new System.ComponentModel.BackgroundWorker();
            this.SuspendLayout();
            // 
            // textBoxServer
            // 
            this.textBoxServer.Location = new System.Drawing.Point(95, 39);
            this.textBoxServer.Name = "textBoxServer";
            this.textBoxServer.Size = new System.Drawing.Size(327, 21);
            this.textBoxServer.TabIndex = 0;
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(95, 12);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(327, 21);
            this.textBoxPort.TabIndex = 1;
            this.textBoxPort.Text = "0";
            // 
            // labelPort
            // 
            this.labelPort.AutoSize = true;
            this.labelPort.Location = new System.Drawing.Point(12, 15);
            this.labelPort.Name = "labelPort";
            this.labelPort.Size = new System.Drawing.Size(77, 12);
            this.labelPort.TabIndex = 2;
            this.labelPort.Text = "监听端口号：";
            // 
            // labelServer
            // 
            this.labelServer.AutoSize = true;
            this.labelServer.Location = new System.Drawing.Point(12, 42);
            this.labelServer.Name = "labelServer";
            this.labelServer.Size = new System.Drawing.Size(77, 12);
            this.labelServer.TabIndex = 3;
            this.labelServer.Text = "服务器地址：";
            // 
            // labelClientAddress
            // 
            this.labelClientAddress.AutoSize = true;
            this.labelClientAddress.Location = new System.Drawing.Point(12, 69);
            this.labelClientAddress.Name = "labelClientAddress";
            this.labelClientAddress.Size = new System.Drawing.Size(77, 12);
            this.labelClientAddress.TabIndex = 4;
            this.labelClientAddress.Text = "客户端地址：";
            // 
            // labelTarget
            // 
            this.labelTarget.AutoSize = true;
            this.labelTarget.Location = new System.Drawing.Point(12, 123);
            this.labelTarget.Name = "labelTarget";
            this.labelTarget.Size = new System.Drawing.Size(65, 12);
            this.labelTarget.TabIndex = 5;
            this.labelTarget.Text = "目标地址：";
            // 
            // textBoxClientAddress
            // 
            this.textBoxClientAddress.Location = new System.Drawing.Point(95, 66);
            this.textBoxClientAddress.Name = "textBoxClientAddress";
            this.textBoxClientAddress.Size = new System.Drawing.Size(327, 21);
            this.textBoxClientAddress.TabIndex = 6;
            // 
            // textBoxTarget
            // 
            this.textBoxTarget.Location = new System.Drawing.Point(95, 120);
            this.textBoxTarget.Name = "textBoxTarget";
            this.textBoxTarget.Size = new System.Drawing.Size(327, 21);
            this.textBoxTarget.TabIndex = 7;
            this.textBoxTarget.Text = "8888";
            // 
            // buttonCancel
            // 
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(14, 376);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 8;
            this.buttonCancel.Text = "取消";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // buttonOK
            // 
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Location = new System.Drawing.Point(347, 376);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(75, 23);
            this.buttonOK.TabIndex = 9;
            this.buttonOK.Text = "确定";
            this.buttonOK.UseVisualStyleBackColor = true;
            // 
            // labelClientName
            // 
            this.labelClientName.AutoSize = true;
            this.labelClientName.Location = new System.Drawing.Point(12, 96);
            this.labelClientName.Name = "labelClientName";
            this.labelClientName.Size = new System.Drawing.Size(77, 12);
            this.labelClientName.TabIndex = 10;
            this.labelClientName.Text = "客户端名称：";
            // 
            // textBoxClientName
            // 
            this.textBoxClientName.Location = new System.Drawing.Point(95, 93);
            this.textBoxClientName.Name = "textBoxClientName";
            this.textBoxClientName.Size = new System.Drawing.Size(327, 21);
            this.textBoxClientName.TabIndex = 11;
            // 
            // listBoxClients
            // 
            this.listBoxClients.FormattingEnabled = true;
            this.listBoxClients.ItemHeight = 12;
            this.listBoxClients.Location = new System.Drawing.Point(14, 147);
            this.listBoxClients.Name = "listBoxClients";
            this.listBoxClients.Size = new System.Drawing.Size(408, 220);
            this.listBoxClients.TabIndex = 12;
            this.listBoxClients.DoubleClick += new System.EventHandler(this.ListBoxClients_DoubleClick);
            // 
            // buttonRefresh
            // 
            this.buttonRefresh.Location = new System.Drawing.Point(181, 376);
            this.buttonRefresh.Name = "buttonRefresh";
            this.buttonRefresh.Size = new System.Drawing.Size(75, 23);
            this.buttonRefresh.TabIndex = 13;
            this.buttonRefresh.Text = "刷新";
            this.buttonRefresh.UseVisualStyleBackColor = true;
            this.buttonRefresh.Click += new System.EventHandler(this.ButtonRefresh_Click);
            // 
            // backgroundWorkerRefresh
            // 
            this.backgroundWorkerRefresh.DoWork += new System.ComponentModel.DoWorkEventHandler(this.BackgroundWorkerRefresh_DoWork);
            this.backgroundWorkerRefresh.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.BackgroundWorkerRefresh_RunWorkerCompleted);
            // 
            // FormAddProxy
            // 
            this.AcceptButton = this.buttonOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(434, 411);
            this.Controls.Add(this.buttonRefresh);
            this.Controls.Add(this.listBoxClients);
            this.Controls.Add(this.textBoxClientName);
            this.Controls.Add(this.labelClientName);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.textBoxTarget);
            this.Controls.Add(this.textBoxClientAddress);
            this.Controls.Add(this.labelTarget);
            this.Controls.Add(this.labelClientAddress);
            this.Controls.Add(this.labelServer);
            this.Controls.Add(this.labelPort);
            this.Controls.Add(this.textBoxPort);
            this.Controls.Add(this.textBoxServer);
            this.Name = "FormAddProxy";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "添加映射";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox textBoxServer;
        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Label labelPort;
        private System.Windows.Forms.Label labelServer;
        private System.Windows.Forms.Label labelClientAddress;
        private System.Windows.Forms.Label labelTarget;
        private System.Windows.Forms.TextBox textBoxClientAddress;
        private System.Windows.Forms.TextBox textBoxTarget;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.Label labelClientName;
        private System.Windows.Forms.TextBox textBoxClientName;
        private System.Windows.Forms.ListBox listBoxClients;
        private System.Windows.Forms.Button buttonRefresh;
        private System.ComponentModel.BackgroundWorker backgroundWorkerRefresh;
    }
}