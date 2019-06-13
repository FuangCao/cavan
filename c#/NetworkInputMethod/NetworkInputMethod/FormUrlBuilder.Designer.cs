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
            this.listView1 = new System.Windows.Forms.ListView();
            this.SuspendLayout();
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(12, 12);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(250, 21);
            this.textBoxPort.TabIndex = 0;
            this.textBoxPort.Text = "8021";
            // 
            // buttonSwitch
            // 
            this.buttonSwitch.Location = new System.Drawing.Point(268, 10);
            this.buttonSwitch.Name = "buttonSwitch";
            this.buttonSwitch.Size = new System.Drawing.Size(75, 23);
            this.buttonSwitch.TabIndex = 1;
            this.buttonSwitch.Text = "打开";
            this.buttonSwitch.UseVisualStyleBackColor = true;
            this.buttonSwitch.Click += new System.EventHandler(this.buttonSwitch_Click);
            // 
            // listView1
            // 
            this.listView1.Location = new System.Drawing.Point(12, 39);
            this.listView1.Name = "listView1";
            this.listView1.Size = new System.Drawing.Size(331, 259);
            this.listView1.TabIndex = 2;
            this.listView1.UseCompatibleStateImageBehavior = false;
            // 
            // FormUrlBuilder
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(355, 310);
            this.Controls.Add(this.listView1);
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
        private System.Windows.Forms.ListView listView1;
    }
}