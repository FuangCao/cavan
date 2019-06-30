namespace NetworkInputMethod
{
    partial class FormHttpCapture
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
            this.textBoxPort = new System.Windows.Forms.TextBox();
            this.buttonSwitch = new System.Windows.Forms.Button();
            this.listBoxUrl = new System.Windows.Forms.ListBox();
            this.textBoxReq = new System.Windows.Forms.TextBox();
            this.textBoxRsp = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(12, 12);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(100, 21);
            this.textBoxPort.TabIndex = 0;
            // 
            // buttonSwitch
            // 
            this.buttonSwitch.Location = new System.Drawing.Point(656, 12);
            this.buttonSwitch.Name = "buttonSwitch";
            this.buttonSwitch.Size = new System.Drawing.Size(75, 23);
            this.buttonSwitch.TabIndex = 1;
            this.buttonSwitch.Text = "启动";
            this.buttonSwitch.UseVisualStyleBackColor = true;
            this.buttonSwitch.Click += new System.EventHandler(this.buttonSwitch_Click);
            // 
            // listBoxUrl
            // 
            this.listBoxUrl.FormattingEnabled = true;
            this.listBoxUrl.ItemHeight = 12;
            this.listBoxUrl.Location = new System.Drawing.Point(12, 39);
            this.listBoxUrl.Name = "listBoxUrl";
            this.listBoxUrl.Size = new System.Drawing.Size(357, 316);
            this.listBoxUrl.TabIndex = 2;
            // 
            // textBoxReq
            // 
            this.textBoxReq.Location = new System.Drawing.Point(375, 41);
            this.textBoxReq.Multiline = true;
            this.textBoxReq.Name = "textBoxReq";
            this.textBoxReq.Size = new System.Drawing.Size(356, 140);
            this.textBoxReq.TabIndex = 3;
            // 
            // textBoxRsp
            // 
            this.textBoxRsp.Location = new System.Drawing.Point(375, 187);
            this.textBoxRsp.Multiline = true;
            this.textBoxRsp.Name = "textBoxRsp";
            this.textBoxRsp.Size = new System.Drawing.Size(356, 168);
            this.textBoxRsp.TabIndex = 4;
            // 
            // FormHttpCapture
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(743, 362);
            this.Controls.Add(this.textBoxRsp);
            this.Controls.Add(this.textBoxReq);
            this.Controls.Add(this.listBoxUrl);
            this.Controls.Add(this.buttonSwitch);
            this.Controls.Add(this.textBoxPort);
            this.Name = "FormHttpCapture";
            this.Text = "抓包工具";
            this.Load += new System.EventHandler(this.FormHttpCapture_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Button buttonSwitch;
        private System.Windows.Forms.ListBox listBoxUrl;
        private System.Windows.Forms.TextBox textBoxReq;
        private System.Windows.Forms.TextBox textBoxRsp;
    }
}