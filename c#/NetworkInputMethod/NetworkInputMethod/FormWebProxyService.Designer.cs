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
            this.textBoxUrl1 = new System.Windows.Forms.TextBox();
            this.textBoxUrl2 = new System.Windows.Forms.TextBox();
            this.textBoxUrl3 = new System.Windows.Forms.TextBox();
            this.buttonProxy1 = new System.Windows.Forms.Button();
            this.buttonProxy2 = new System.Windows.Forms.Button();
            this.buttonProxy3 = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // textBoxUrl
            // 
            this.textBoxUrl.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxUrl.Location = new System.Drawing.Point(12, 14);
            this.textBoxUrl.Name = "textBoxUrl";
            this.textBoxUrl.Size = new System.Drawing.Size(429, 21);
            this.textBoxUrl.TabIndex = 0;
            this.textBoxUrl.Text = "http://www.baidu.com";
            // 
            // textBoxPort
            // 
            this.textBoxPort.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxPort.Location = new System.Drawing.Point(447, 14);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(44, 21);
            this.textBoxPort.TabIndex = 1;
            this.textBoxPort.Text = "9090";
            // 
            // buttonStartStop
            // 
            this.buttonStartStop.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonStartStop.Location = new System.Drawing.Point(497, 12);
            this.buttonStartStop.Name = "buttonStartStop";
            this.buttonStartStop.Size = new System.Drawing.Size(75, 23);
            this.buttonStartStop.TabIndex = 2;
            this.buttonStartStop.Text = "启动";
            this.buttonStartStop.UseVisualStyleBackColor = true;
            this.buttonStartStop.Click += new System.EventHandler(this.buttonStartStop_Click);
            // 
            // listBoxClients
            // 
            this.listBoxClients.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.listBoxClients.FormattingEnabled = true;
            this.listBoxClients.ItemHeight = 12;
            this.listBoxClients.Location = new System.Drawing.Point(12, 137);
            this.listBoxClients.Name = "listBoxClients";
            this.listBoxClients.Size = new System.Drawing.Size(560, 400);
            this.listBoxClients.TabIndex = 3;
            // 
            // textBoxUrl1
            // 
            this.textBoxUrl1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxUrl1.Location = new System.Drawing.Point(12, 42);
            this.textBoxUrl1.Name = "textBoxUrl1";
            this.textBoxUrl1.Size = new System.Drawing.Size(479, 21);
            this.textBoxUrl1.TabIndex = 4;
            // 
            // textBoxUrl2
            // 
            this.textBoxUrl2.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxUrl2.Location = new System.Drawing.Point(12, 71);
            this.textBoxUrl2.Name = "textBoxUrl2";
            this.textBoxUrl2.Size = new System.Drawing.Size(479, 21);
            this.textBoxUrl2.TabIndex = 5;
            // 
            // textBoxUrl3
            // 
            this.textBoxUrl3.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxUrl3.Location = new System.Drawing.Point(12, 100);
            this.textBoxUrl3.Name = "textBoxUrl3";
            this.textBoxUrl3.Size = new System.Drawing.Size(479, 21);
            this.textBoxUrl3.TabIndex = 6;
            // 
            // buttonProxy1
            // 
            this.buttonProxy1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonProxy1.Location = new System.Drawing.Point(497, 41);
            this.buttonProxy1.Name = "buttonProxy1";
            this.buttonProxy1.Size = new System.Drawing.Size(75, 23);
            this.buttonProxy1.TabIndex = 7;
            this.buttonProxy1.Text = "代理";
            this.buttonProxy1.UseVisualStyleBackColor = true;
            this.buttonProxy1.Click += new System.EventHandler(this.buttonProxy1_Click);
            // 
            // buttonProxy2
            // 
            this.buttonProxy2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonProxy2.Location = new System.Drawing.Point(497, 70);
            this.buttonProxy2.Name = "buttonProxy2";
            this.buttonProxy2.Size = new System.Drawing.Size(75, 23);
            this.buttonProxy2.TabIndex = 8;
            this.buttonProxy2.Text = "代理";
            this.buttonProxy2.UseVisualStyleBackColor = true;
            this.buttonProxy2.Click += new System.EventHandler(this.buttonProxy2_Click);
            // 
            // buttonProxy3
            // 
            this.buttonProxy3.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonProxy3.Location = new System.Drawing.Point(497, 99);
            this.buttonProxy3.Name = "buttonProxy3";
            this.buttonProxy3.Size = new System.Drawing.Size(75, 23);
            this.buttonProxy3.TabIndex = 9;
            this.buttonProxy3.Text = "代理";
            this.buttonProxy3.UseVisualStyleBackColor = true;
            this.buttonProxy3.Click += new System.EventHandler(this.buttonProxy3_Click);
            // 
            // FormWebProxyService
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 561);
            this.Controls.Add(this.buttonProxy3);
            this.Controls.Add(this.buttonProxy2);
            this.Controls.Add(this.buttonProxy1);
            this.Controls.Add(this.textBoxUrl3);
            this.Controls.Add(this.textBoxUrl2);
            this.Controls.Add(this.textBoxUrl1);
            this.Controls.Add(this.listBoxClients);
            this.Controls.Add(this.buttonStartStop);
            this.Controls.Add(this.textBoxPort);
            this.Controls.Add(this.textBoxUrl);
            this.Name = "FormWebProxyService";
            this.Text = "网页代理";
            this.Load += new System.EventHandler(this.FormWebProxyService_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox textBoxUrl;
        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Button buttonStartStop;
        private System.Windows.Forms.ListBox listBoxClients;
        private System.Windows.Forms.TextBox textBoxUrl1;
        private System.Windows.Forms.TextBox textBoxUrl2;
        private System.Windows.Forms.TextBox textBoxUrl3;
        private System.Windows.Forms.Button buttonProxy1;
        private System.Windows.Forms.Button buttonProxy2;
        private System.Windows.Forms.Button buttonProxy3;
    }
}