namespace NetworkInputMethod
{
    partial class FormUrlBuilder
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
            this.listViewUrls = new System.Windows.Forms.ListView();
            this.columnHeaderIndex = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeaderUrl = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.buttonGenerate = new System.Windows.Forms.Button();
            this.buttonClear = new System.Windows.Forms.Button();
            this.columnHeaderName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.SuspendLayout();
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(12, 12);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(39, 21);
            this.textBoxPort.TabIndex = 0;
            this.textBoxPort.Text = "8029";
            // 
            // buttonSwitch
            // 
            this.buttonSwitch.Location = new System.Drawing.Point(57, 10);
            this.buttonSwitch.Name = "buttonSwitch";
            this.buttonSwitch.Size = new System.Drawing.Size(75, 23);
            this.buttonSwitch.TabIndex = 1;
            this.buttonSwitch.Text = "启动";
            this.buttonSwitch.UseVisualStyleBackColor = true;
            this.buttonSwitch.Click += new System.EventHandler(this.buttonSwitch_Click);
            // 
            // listViewUrls
            // 
            this.listViewUrls.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeaderIndex,
            this.columnHeaderName,
            this.columnHeaderUrl});
            this.listViewUrls.FullRowSelect = true;
            this.listViewUrls.GridLines = true;
            this.listViewUrls.Location = new System.Drawing.Point(12, 39);
            this.listViewUrls.Name = "listViewUrls";
            this.listViewUrls.Size = new System.Drawing.Size(571, 306);
            this.listViewUrls.TabIndex = 2;
            this.listViewUrls.UseCompatibleStateImageBehavior = false;
            this.listViewUrls.View = System.Windows.Forms.View.Details;
            // 
            // columnHeaderIndex
            // 
            this.columnHeaderIndex.Text = "序号";
            // 
            // columnHeaderUrl
            // 
            this.columnHeaderUrl.Text = "URL";
            this.columnHeaderUrl.Width = 509;
            // 
            // buttonGenerate
            // 
            this.buttonGenerate.Location = new System.Drawing.Point(508, 10);
            this.buttonGenerate.Name = "buttonGenerate";
            this.buttonGenerate.Size = new System.Drawing.Size(75, 23);
            this.buttonGenerate.TabIndex = 3;
            this.buttonGenerate.Text = "生成";
            this.buttonGenerate.UseVisualStyleBackColor = true;
            this.buttonGenerate.Click += new System.EventHandler(this.buttonGenerate_Click);
            // 
            // buttonClear
            // 
            this.buttonClear.Location = new System.Drawing.Point(427, 10);
            this.buttonClear.Name = "buttonClear";
            this.buttonClear.Size = new System.Drawing.Size(75, 23);
            this.buttonClear.TabIndex = 4;
            this.buttonClear.Text = "清空";
            this.buttonClear.UseVisualStyleBackColor = true;
            this.buttonClear.Click += new System.EventHandler(this.buttonClear_Click);
            // 
            // columnHeaderName
            // 
            this.columnHeaderName.Text = "名称";
            // 
            // FormUrlBuilder
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(595, 357);
            this.Controls.Add(this.buttonClear);
            this.Controls.Add(this.buttonGenerate);
            this.Controls.Add(this.listViewUrls);
            this.Controls.Add(this.buttonSwitch);
            this.Controls.Add(this.textBoxPort);
            this.Name = "FormUrlBuilder";
            this.Text = "FormUrlBuilder";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Button buttonSwitch;
        private System.Windows.Forms.ListView listViewUrls;
        private System.Windows.Forms.ColumnHeader columnHeaderIndex;
        private System.Windows.Forms.ColumnHeader columnHeaderUrl;
        private System.Windows.Forms.Button buttonGenerate;
        private System.Windows.Forms.Button buttonClear;
        private System.Windows.Forms.ColumnHeader columnHeaderName;
    }
}