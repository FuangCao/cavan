namespace NetworkInputMethod
{
    partial class FormAlipay
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
            this.textBoxCodes = new System.Windows.Forms.TextBox();
            this.buttonSend = new System.Windows.Forms.Button();
            this.buttonOpen = new System.Windows.Forms.Button();
            this.buttonClear = new System.Windows.Forms.Button();
            this.checkBoxClipboardSend = new System.Windows.Forms.CheckBox();
            this.checkBoxClipboardListen = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // textBoxCodes
            // 
            this.textBoxCodes.Location = new System.Drawing.Point(12, 12);
            this.textBoxCodes.Multiline = true;
            this.textBoxCodes.Name = "textBoxCodes";
            this.textBoxCodes.Size = new System.Drawing.Size(320, 235);
            this.textBoxCodes.TabIndex = 0;
            // 
            // buttonSend
            // 
            this.buttonSend.Location = new System.Drawing.Point(257, 286);
            this.buttonSend.Name = "buttonSend";
            this.buttonSend.Size = new System.Drawing.Size(75, 23);
            this.buttonSend.TabIndex = 2;
            this.buttonSend.Text = "发送";
            this.buttonSend.UseVisualStyleBackColor = true;
            this.buttonSend.Click += new System.EventHandler(this.buttonSend_Click);
            // 
            // buttonOpen
            // 
            this.buttonOpen.Location = new System.Drawing.Point(12, 286);
            this.buttonOpen.Name = "buttonOpen";
            this.buttonOpen.Size = new System.Drawing.Size(75, 23);
            this.buttonOpen.TabIndex = 3;
            this.buttonOpen.Text = "打开";
            this.buttonOpen.UseVisualStyleBackColor = true;
            this.buttonOpen.Click += new System.EventHandler(this.buttonOpen_Click);
            // 
            // buttonClear
            // 
            this.buttonClear.Location = new System.Drawing.Point(93, 286);
            this.buttonClear.Name = "buttonClear";
            this.buttonClear.Size = new System.Drawing.Size(75, 23);
            this.buttonClear.TabIndex = 4;
            this.buttonClear.Text = "清理";
            this.buttonClear.UseVisualStyleBackColor = true;
            this.buttonClear.Click += new System.EventHandler(this.buttonClear_Click);
            // 
            // checkBoxClipboardSend
            // 
            this.checkBoxClipboardSend.AutoSize = true;
            this.checkBoxClipboardSend.Location = new System.Drawing.Point(248, 253);
            this.checkBoxClipboardSend.Name = "checkBoxClipboardSend";
            this.checkBoxClipboardSend.Size = new System.Drawing.Size(84, 16);
            this.checkBoxClipboardSend.TabIndex = 5;
            this.checkBoxClipboardSend.Text = "剪切板发送";
            this.checkBoxClipboardSend.UseVisualStyleBackColor = true;
            // 
            // checkBoxClipboardListen
            // 
            this.checkBoxClipboardListen.AutoSize = true;
            this.checkBoxClipboardListen.Location = new System.Drawing.Point(12, 253);
            this.checkBoxClipboardListen.Name = "checkBoxClipboardListen";
            this.checkBoxClipboardListen.Size = new System.Drawing.Size(84, 16);
            this.checkBoxClipboardListen.TabIndex = 6;
            this.checkBoxClipboardListen.Text = "剪切板监听";
            this.checkBoxClipboardListen.UseVisualStyleBackColor = true;
            // 
            // FormAlipay
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(344, 321);
            this.Controls.Add(this.checkBoxClipboardListen);
            this.Controls.Add(this.checkBoxClipboardSend);
            this.Controls.Add(this.buttonClear);
            this.Controls.Add(this.buttonOpen);
            this.Controls.Add(this.buttonSend);
            this.Controls.Add(this.textBoxCodes);
            this.Name = "FormAlipay";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Alipay";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox textBoxCodes;
        private System.Windows.Forms.Button buttonSend;
        private System.Windows.Forms.Button buttonOpen;
        private System.Windows.Forms.Button buttonClear;
        private System.Windows.Forms.CheckBox checkBoxClipboardSend;
        private System.Windows.Forms.CheckBox checkBoxClipboardListen;
    }
}