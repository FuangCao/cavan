namespace NetworkInputMethod
{
    partial class FormWebProxyService
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
            this.textBoxUrl = new System.Windows.Forms.TextBox();
            this.textBoxPort = new System.Windows.Forms.TextBox();
            this.buttonStartStop = new System.Windows.Forms.Button();
            this.listBoxClients = new System.Windows.Forms.ListBox();
            this.SuspendLayout();
            // 
            // textBoxUrl
            // 
            this.textBoxUrl.Location = new System.Drawing.Point(12, 14);
            this.textBoxUrl.Name = "textBoxUrl";
            this.textBoxUrl.Size = new System.Drawing.Size(182, 21);
            this.textBoxUrl.TabIndex = 0;
            this.textBoxUrl.Text = "http://www.baidu.com";
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(200, 14);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(44, 21);
            this.textBoxPort.TabIndex = 1;
            this.textBoxPort.Text = "9090";
            // 
            // buttonStartStop
            // 
            this.buttonStartStop.Location = new System.Drawing.Point(250, 12);
            this.buttonStartStop.Name = "buttonStartStop";
            this.buttonStartStop.Size = new System.Drawing.Size(75, 23);
            this.buttonStartStop.TabIndex = 2;
            this.buttonStartStop.Text = "启动";
            this.buttonStartStop.UseVisualStyleBackColor = true;
            this.buttonStartStop.Click += new System.EventHandler(this.buttonStartStop_Click);
            // 
            // listBoxClients
            // 
            this.listBoxClients.FormattingEnabled = true;
            this.listBoxClients.ItemHeight = 12;
            this.listBoxClients.Location = new System.Drawing.Point(12, 41);
            this.listBoxClients.Name = "listBoxClients";
            this.listBoxClients.Size = new System.Drawing.Size(313, 316);
            this.listBoxClients.TabIndex = 3;
            // 
            // FormWebProxyService
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(337, 376);
            this.Controls.Add(this.listBoxClients);
            this.Controls.Add(this.buttonStartStop);
            this.Controls.Add(this.textBoxPort);
            this.Controls.Add(this.textBoxUrl);
            this.Name = "FormWebProxyService";
            this.Text = "WebProxyService";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.FormWebProxyService_FormClosing);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox textBoxUrl;
        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Button buttonStartStop;
        private System.Windows.Forms.ListBox listBoxClients;
    }
}