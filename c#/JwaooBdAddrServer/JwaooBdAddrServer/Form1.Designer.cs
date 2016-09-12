namespace JwaooBdAddrServer
{
    partial class Form1
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
        /// 设计器支持所需的方法 - 不要修改
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            this.buttonStart = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.listViewClients = new System.Windows.Forms.ListView();
            this.columnHeaderClient = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.buttonStop = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.textBoxIp = new System.Windows.Forms.TextBox();
            this.textBoxPort = new System.Windows.Forms.TextBox();
            this.labelState = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.labelNextAddress = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // buttonStart
            // 
            this.buttonStart.Location = new System.Drawing.Point(4, 359);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(75, 23);
            this.buttonStart.TabIndex = 0;
            this.buttonStart.Text = "启动";
            this.buttonStart.UseVisualStyleBackColor = true;
            this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(2, 297);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(77, 12);
            this.label1.TabIndex = 1;
            this.label1.Text = "本机IP地址：";
            // 
            // listViewClients
            // 
            this.listViewClients.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderClient});
            this.listViewClients.GridLines = true;
            this.listViewClients.Location = new System.Drawing.Point(2, 2);
            this.listViewClients.Name = "listViewClients";
            this.listViewClients.Size = new System.Drawing.Size(280, 234);
            this.listViewClients.TabIndex = 3;
            this.listViewClients.UseCompatibleStateImageBehavior = false;
            this.listViewClients.View = System.Windows.Forms.View.Details;
            // 
            // columnHeaderClient
            // 
            this.columnHeaderClient.Text = "客户端列表";
            this.columnHeaderClient.Width = 276;
            // 
            // buttonStop
            // 
            this.buttonStop.Location = new System.Drawing.Point(197, 360);
            this.buttonStop.Name = "buttonStop";
            this.buttonStop.Size = new System.Drawing.Size(75, 23);
            this.buttonStop.TabIndex = 4;
            this.buttonStop.Text = "停止";
            this.buttonStop.UseVisualStyleBackColor = true;
            this.buttonStop.Click += new System.EventHandler(this.buttonStop_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(2, 325);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(77, 12);
            this.label2.TabIndex = 5;
            this.label2.Text = "服务端口号：";
            // 
            // textBoxIp
            // 
            this.textBoxIp.Location = new System.Drawing.Point(93, 295);
            this.textBoxIp.Name = "textBoxIp";
            this.textBoxIp.ReadOnly = true;
            this.textBoxIp.Size = new System.Drawing.Size(189, 21);
            this.textBoxIp.TabIndex = 6;
            this.textBoxIp.Click += new System.EventHandler(this.textBoxIp_Click);
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(93, 322);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(189, 21);
            this.textBoxPort.TabIndex = 7;
            this.textBoxPort.Text = "12345";
            // 
            // labelState
            // 
            this.labelState.AutoSize = true;
            this.labelState.Location = new System.Drawing.Point(0, 239);
            this.labelState.Name = "labelState";
            this.labelState.Size = new System.Drawing.Size(77, 12);
            this.labelState.TabIndex = 8;
            this.labelState.Text = "服务器未启动";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(2, 269);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(77, 12);
            this.label3.TabIndex = 9;
            this.label3.Text = "下一个地址：";
            // 
            // labelNextAddress
            // 
            this.labelNextAddress.AutoSize = true;
            this.labelNextAddress.Location = new System.Drawing.Point(91, 269);
            this.labelNextAddress.Name = "labelNextAddress";
            this.labelNextAddress.Size = new System.Drawing.Size(11, 12);
            this.labelNextAddress.TabIndex = 10;
            this.labelNextAddress.Text = "0";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 389);
            this.Controls.Add(this.labelNextAddress);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.labelState);
            this.Controls.Add(this.textBoxPort);
            this.Controls.Add(this.textBoxIp);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.buttonStop);
            this.Controls.Add(this.listViewClients);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.buttonStart);
            this.Name = "Form1";
            this.Text = "井蛙蓝牙硬件地址服务器";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ListView listViewClients;
        private System.Windows.Forms.Button buttonStop;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox textBoxIp;
        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Label labelState;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label labelNextAddress;
        private System.Windows.Forms.ColumnHeader columnHeaderClient;
    }
}

