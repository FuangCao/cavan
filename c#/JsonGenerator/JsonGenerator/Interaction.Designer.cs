namespace JsonGenerator {
    partial class Interaction {
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
            this.components = new System.ComponentModel.Container();
            this.buttonOK = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.buttonSubactionAdd = new System.Windows.Forms.Button();
            this.textBoxFov = new System.Windows.Forms.TextBox();
            this.textBoxName = new System.Windows.Forms.TextBox();
            this.textBoxVideo = new System.Windows.Forms.TextBox();
            this.openFileDialogMp4 = new System.Windows.Forms.OpenFileDialog();
            this.buttonListViewSubaction = new JsonGenerator.ButtonListView();
            this.contextMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.removeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.buttonDelete = new System.Windows.Forms.Button();
            this.contextMenuStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // buttonOK
            // 
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Location = new System.Drawing.Point(12, 177);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(75, 23);
            this.buttonOK.TabIndex = 0;
            this.buttonOK.Text = "OK";
            this.buttonOK.UseVisualStyleBackColor = true;
            // 
            // buttonCancel
            // 
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(796, 177);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 1;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(10, 16);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(29, 12);
            this.label1.TabIndex = 2;
            this.label1.Text = "Fov:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(10, 53);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(35, 12);
            this.label2.TabIndex = 3;
            this.label2.Text = "Name:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(10, 90);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(41, 12);
            this.label3.TabIndex = 4;
            this.label3.Text = "Video:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(10, 128);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(65, 12);
            this.label4.TabIndex = 5;
            this.label4.Text = "Subaction:";
            // 
            // buttonSubactionAdd
            // 
            this.buttonSubactionAdd.Location = new System.Drawing.Point(81, 123);
            this.buttonSubactionAdd.Name = "buttonSubactionAdd";
            this.buttonSubactionAdd.Size = new System.Drawing.Size(32, 23);
            this.buttonSubactionAdd.TabIndex = 7;
            this.buttonSubactionAdd.Text = "Add";
            this.buttonSubactionAdd.UseVisualStyleBackColor = true;
            this.buttonSubactionAdd.Click += new System.EventHandler(this.buttonSubactionAdd_Click);
            // 
            // textBoxFov
            // 
            this.textBoxFov.Location = new System.Drawing.Point(82, 12);
            this.textBoxFov.Name = "textBoxFov";
            this.textBoxFov.Size = new System.Drawing.Size(790, 21);
            this.textBoxFov.TabIndex = 8;
            this.textBoxFov.Text = "0";
            // 
            // textBoxName
            // 
            this.textBoxName.Location = new System.Drawing.Point(81, 49);
            this.textBoxName.Name = "textBoxName";
            this.textBoxName.Size = new System.Drawing.Size(791, 21);
            this.textBoxName.TabIndex = 9;
            // 
            // textBoxVideo
            // 
            this.textBoxVideo.Location = new System.Drawing.Point(82, 86);
            this.textBoxVideo.Name = "textBoxVideo";
            this.textBoxVideo.ReadOnly = true;
            this.textBoxVideo.Size = new System.Drawing.Size(790, 21);
            this.textBoxVideo.TabIndex = 10;
            this.textBoxVideo.Click += new System.EventHandler(this.textBoxVideo_Click);
            // 
            // openFileDialogMp4
            // 
            this.openFileDialogMp4.DefaultExt = "mp4";
            this.openFileDialogMp4.Filter = "Video|*.mp4|All|*.*";
            // 
            // buttonListViewSubaction
            // 
            this.buttonListViewSubaction.Location = new System.Drawing.Point(119, 123);
            this.buttonListViewSubaction.Name = "buttonListViewSubaction";
            this.buttonListViewSubaction.Size = new System.Drawing.Size(753, 23);
            this.buttonListViewSubaction.TabIndex = 6;
            // 
            // contextMenuStrip
            // 
            this.contextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.removeToolStripMenuItem});
            this.contextMenuStrip.Name = "contextMenuStrip";
            this.contextMenuStrip.Size = new System.Drawing.Size(114, 26);
            // 
            // removeToolStripMenuItem
            // 
            this.removeToolStripMenuItem.Name = "removeToolStripMenuItem";
            this.removeToolStripMenuItem.Size = new System.Drawing.Size(113, 22);
            this.removeToolStripMenuItem.Text = "Delete";
            this.removeToolStripMenuItem.Click += new System.EventHandler(this.removeToolStripMenuItem_Click);
            // 
            // buttonDelete
            // 
            this.buttonDelete.DialogResult = System.Windows.Forms.DialogResult.No;
            this.buttonDelete.Location = new System.Drawing.Point(404, 177);
            this.buttonDelete.Name = "buttonDelete";
            this.buttonDelete.Size = new System.Drawing.Size(75, 23);
            this.buttonDelete.TabIndex = 11;
            this.buttonDelete.Text = "Delete";
            this.buttonDelete.UseVisualStyleBackColor = true;
            // 
            // Interaction
            // 
            this.AcceptButton = this.buttonOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(884, 212);
            this.Controls.Add(this.buttonDelete);
            this.Controls.Add(this.textBoxVideo);
            this.Controls.Add(this.textBoxName);
            this.Controls.Add(this.textBoxFov);
            this.Controls.Add(this.buttonSubactionAdd);
            this.Controls.Add(this.buttonListViewSubaction);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.buttonOK);
            this.Name = "Interaction";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Interaction";
            this.contextMenuStrip.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private ButtonListView buttonListViewSubaction;
        private System.Windows.Forms.Button buttonSubactionAdd;
        private System.Windows.Forms.TextBox textBoxFov;
        private System.Windows.Forms.TextBox textBoxName;
        private System.Windows.Forms.TextBox textBoxVideo;
        private System.Windows.Forms.OpenFileDialog openFileDialogMp4;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem removeToolStripMenuItem;
        private System.Windows.Forms.Button buttonDelete;
    }
}