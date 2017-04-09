namespace JwaooOtpProgrammer {
    partial class JwaooMacAddressEditDialog {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing) {
            if (disposing && (components != null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent() {
            this.labelAddressStart = new System.Windows.Forms.Label();
            this.labelAddressCount = new System.Windows.Forms.Label();
            this.textBoxAddressStart = new System.Windows.Forms.TextBox();
            this.textBoxAddressCount = new System.Windows.Forms.TextBox();
            this.buttonOK = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // labelAddressStart
            // 
            this.labelAddressStart.AutoSize = true;
            this.labelAddressStart.Location = new System.Drawing.Point(12, 15);
            this.labelAddressStart.Name = "labelAddressStart";
            this.labelAddressStart.Size = new System.Drawing.Size(65, 12);
            this.labelAddressStart.TabIndex = 0;
            this.labelAddressStart.Text = "起始地址：";
            // 
            // labelAddressCount
            // 
            this.labelAddressCount.AutoSize = true;
            this.labelAddressCount.Location = new System.Drawing.Point(12, 42);
            this.labelAddressCount.Name = "labelAddressCount";
            this.labelAddressCount.Size = new System.Drawing.Size(65, 12);
            this.labelAddressCount.TabIndex = 1;
            this.labelAddressCount.Text = "地址个数：";
            // 
            // textBoxAddressStart
            // 
            this.textBoxAddressStart.Location = new System.Drawing.Point(85, 12);
            this.textBoxAddressStart.Name = "textBoxAddressStart";
            this.textBoxAddressStart.Size = new System.Drawing.Size(158, 21);
            this.textBoxAddressStart.TabIndex = 2;
            // 
            // textBoxAddressCount
            // 
            this.textBoxAddressCount.Location = new System.Drawing.Point(85, 39);
            this.textBoxAddressCount.Name = "textBoxAddressCount";
            this.textBoxAddressCount.Size = new System.Drawing.Size(158, 21);
            this.textBoxAddressCount.TabIndex = 3;
            // 
            // buttonOK
            // 
            this.buttonOK.Location = new System.Drawing.Point(13, 74);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(75, 23);
            this.buttonOK.TabIndex = 4;
            this.buttonOK.Text = "确定";
            this.buttonOK.UseVisualStyleBackColor = true;
            this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
            // 
            // buttonCancel
            // 
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(168, 74);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 5;
            this.buttonCancel.Text = "取消";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // JwaooMacAddressEditDialog
            // 
            this.AcceptButton = this.buttonOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(255, 109);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.textBoxAddressCount);
            this.Controls.Add(this.textBoxAddressStart);
            this.Controls.Add(this.labelAddressCount);
            this.Controls.Add(this.labelAddressStart);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.Name = "JwaooMacAddressEditDialog";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "井蛙MAC地址编辑";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label labelAddressStart;
        private System.Windows.Forms.Label labelAddressCount;
        private System.Windows.Forms.TextBox textBoxAddressStart;
        private System.Windows.Forms.TextBox textBoxAddressCount;
        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.Button buttonCancel;
    }
}