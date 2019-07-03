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
            this.tableLayoutPanelDetail = new System.Windows.Forms.TableLayoutPanel();
            this.textBoxRsp = new System.Windows.Forms.TextBox();
            this.tableLayoutPanelDetail.SuspendLayout();
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
            this.buttonSwitch.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.buttonSwitch.Location = new System.Drawing.Point(797, 10);
            this.buttonSwitch.Name = "buttonSwitch";
            this.buttonSwitch.Size = new System.Drawing.Size(75, 23);
            this.buttonSwitch.TabIndex = 1;
            this.buttonSwitch.Text = "启动";
            this.buttonSwitch.UseVisualStyleBackColor = true;
            this.buttonSwitch.Click += new System.EventHandler(this.buttonSwitch_Click);
            // 
            // listBoxUrl
            // 
            this.listBoxUrl.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.listBoxUrl.FormattingEnabled = true;
            this.listBoxUrl.ItemHeight = 12;
            this.listBoxUrl.Location = new System.Drawing.Point(12, 42);
            this.listBoxUrl.Name = "listBoxUrl";
            this.listBoxUrl.Size = new System.Drawing.Size(323, 508);
            this.listBoxUrl.TabIndex = 2;
            this.listBoxUrl.SelectedIndexChanged += new System.EventHandler(this.listBoxUrl_SelectedIndexChanged);
            // 
            // textBoxReq
            // 
            this.textBoxReq.Dock = System.Windows.Forms.DockStyle.Fill;
            this.textBoxReq.Location = new System.Drawing.Point(3, 3);
            this.textBoxReq.Multiline = true;
            this.textBoxReq.Name = "textBoxReq";
            this.textBoxReq.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBoxReq.Size = new System.Drawing.Size(525, 248);
            this.textBoxReq.TabIndex = 3;
            this.textBoxReq.WordWrap = false;
            // 
            // tableLayoutPanelDetail
            // 
            this.tableLayoutPanelDetail.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tableLayoutPanelDetail.ColumnCount = 1;
            this.tableLayoutPanelDetail.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanelDetail.Controls.Add(this.textBoxReq, 0, 0);
            this.tableLayoutPanelDetail.Controls.Add(this.textBoxRsp, 0, 1);
            this.tableLayoutPanelDetail.Location = new System.Drawing.Point(341, 42);
            this.tableLayoutPanelDetail.Name = "tableLayoutPanelDetail";
            this.tableLayoutPanelDetail.RowCount = 2;
            this.tableLayoutPanelDetail.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanelDetail.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tableLayoutPanelDetail.Size = new System.Drawing.Size(531, 508);
            this.tableLayoutPanelDetail.TabIndex = 5;
            // 
            // textBoxRsp
            // 
            this.textBoxRsp.Dock = System.Windows.Forms.DockStyle.Fill;
            this.textBoxRsp.Location = new System.Drawing.Point(3, 257);
            this.textBoxRsp.Multiline = true;
            this.textBoxRsp.Name = "textBoxRsp";
            this.textBoxRsp.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBoxRsp.Size = new System.Drawing.Size(525, 248);
            this.textBoxRsp.TabIndex = 4;
            this.textBoxRsp.WordWrap = false;
            // 
            // FormHttpCapture
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(884, 561);
            this.Controls.Add(this.tableLayoutPanelDetail);
            this.Controls.Add(this.listBoxUrl);
            this.Controls.Add(this.buttonSwitch);
            this.Controls.Add(this.textBoxPort);
            this.Name = "FormHttpCapture";
            this.Text = "抓包工具";
            this.Load += new System.EventHandler(this.FormHttpCapture_Load);
            this.tableLayoutPanelDetail.ResumeLayout(false);
            this.tableLayoutPanelDetail.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Button buttonSwitch;
        private System.Windows.Forms.ListBox listBoxUrl;
        private System.Windows.Forms.TextBox textBoxReq;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanelDetail;
        private System.Windows.Forms.TextBox textBoxRsp;
    }
}