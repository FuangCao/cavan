namespace JsonGenerator {
    partial class Subaction {
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
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.buttonVideoAdd = new System.Windows.Forms.Button();
            this.buttonAudioMidAdd = new System.Windows.Forms.Button();
            this.buttonAudioHighAdd = new System.Windows.Forms.Button();
            this.buttonAudioLowAdd = new System.Windows.Forms.Button();
            this.buttonAudioIdleAdd = new System.Windows.Forms.Button();
            this.buttonListViewVideo = new JsonGenerator.ButtonListView();
            this.buttonListViewAudioHigh = new JsonGenerator.ButtonListView();
            this.buttonListViewAudioMid = new JsonGenerator.ButtonListView();
            this.buttonListViewAudioIdle = new JsonGenerator.ButtonListView();
            this.buttonListViewAudioLow = new JsonGenerator.ButtonListView();
            this.buttonOK = new System.Windows.Forms.Button();
            this.buttonCancel = new System.Windows.Forms.Button();
            this.openFileDialogH264 = new System.Windows.Forms.OpenFileDialog();
            this.openFileDialogMp3 = new System.Windows.Forms.OpenFileDialog();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.contextMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.removeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.buttonDelete = new System.Windows.Forms.Button();
            this.label6 = new System.Windows.Forms.Label();
            this.buttonVideoIdleAdd = new System.Windows.Forms.Button();
            this.buttonListViewVideoIdle = new JsonGenerator.ButtonListView();
            this.contextMenuStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 17);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(41, 12);
            this.label1.TabIndex = 0;
            this.label1.Text = "Video:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 93);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(71, 12);
            this.label2.TabIndex = 1;
            this.label2.Text = "Audio Idle:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 131);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(65, 12);
            this.label3.TabIndex = 2;
            this.label3.Text = "Audio Low:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(12, 169);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(65, 12);
            this.label4.TabIndex = 3;
            this.label4.Text = "Audio Mid:";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(12, 207);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(71, 12);
            this.label5.TabIndex = 4;
            this.label5.Text = "Audio High:";
            // 
            // buttonVideoAdd
            // 
            this.buttonVideoAdd.Location = new System.Drawing.Point(89, 12);
            this.buttonVideoAdd.Name = "buttonVideoAdd";
            this.buttonVideoAdd.Size = new System.Drawing.Size(32, 23);
            this.buttonVideoAdd.TabIndex = 5;
            this.buttonVideoAdd.Text = "Add";
            this.buttonVideoAdd.UseVisualStyleBackColor = true;
            this.buttonVideoAdd.Click += new System.EventHandler(this.buttonVideoAdd_Click);
            // 
            // buttonAudioMidAdd
            // 
            this.buttonAudioMidAdd.Location = new System.Drawing.Point(89, 164);
            this.buttonAudioMidAdd.Name = "buttonAudioMidAdd";
            this.buttonAudioMidAdd.Size = new System.Drawing.Size(32, 23);
            this.buttonAudioMidAdd.TabIndex = 6;
            this.buttonAudioMidAdd.Text = "Add";
            this.buttonAudioMidAdd.UseVisualStyleBackColor = true;
            this.buttonAudioMidAdd.Click += new System.EventHandler(this.buttonAudioMidAdd_Click);
            // 
            // buttonAudioHighAdd
            // 
            this.buttonAudioHighAdd.Location = new System.Drawing.Point(89, 202);
            this.buttonAudioHighAdd.Name = "buttonAudioHighAdd";
            this.buttonAudioHighAdd.Size = new System.Drawing.Size(32, 23);
            this.buttonAudioHighAdd.TabIndex = 7;
            this.buttonAudioHighAdd.Text = "Add";
            this.buttonAudioHighAdd.UseVisualStyleBackColor = true;
            this.buttonAudioHighAdd.Click += new System.EventHandler(this.buttonAudioHighAdd_Click);
            // 
            // buttonAudioLowAdd
            // 
            this.buttonAudioLowAdd.Location = new System.Drawing.Point(89, 126);
            this.buttonAudioLowAdd.Name = "buttonAudioLowAdd";
            this.buttonAudioLowAdd.Size = new System.Drawing.Size(32, 23);
            this.buttonAudioLowAdd.TabIndex = 8;
            this.buttonAudioLowAdd.Text = "Add";
            this.buttonAudioLowAdd.UseVisualStyleBackColor = true;
            this.buttonAudioLowAdd.Click += new System.EventHandler(this.buttonAudioLowAdd_Click);
            // 
            // buttonAudioIdleAdd
            // 
            this.buttonAudioIdleAdd.Location = new System.Drawing.Point(89, 88);
            this.buttonAudioIdleAdd.Name = "buttonAudioIdleAdd";
            this.buttonAudioIdleAdd.Size = new System.Drawing.Size(32, 23);
            this.buttonAudioIdleAdd.TabIndex = 9;
            this.buttonAudioIdleAdd.Text = "Add";
            this.buttonAudioIdleAdd.UseVisualStyleBackColor = true;
            this.buttonAudioIdleAdd.Click += new System.EventHandler(this.buttonAudioIdleAdd_Click);
            // 
            // buttonListViewVideo
            // 
            this.buttonListViewVideo.Location = new System.Drawing.Point(127, 12);
            this.buttonListViewVideo.Name = "buttonListViewVideo";
            this.buttonListViewVideo.Size = new System.Drawing.Size(745, 23);
            this.buttonListViewVideo.TabIndex = 10;
            // 
            // buttonListViewAudioHigh
            // 
            this.buttonListViewAudioHigh.Location = new System.Drawing.Point(127, 202);
            this.buttonListViewAudioHigh.Name = "buttonListViewAudioHigh";
            this.buttonListViewAudioHigh.Size = new System.Drawing.Size(745, 23);
            this.buttonListViewAudioHigh.TabIndex = 11;
            // 
            // buttonListViewAudioMid
            // 
            this.buttonListViewAudioMid.Location = new System.Drawing.Point(127, 164);
            this.buttonListViewAudioMid.Name = "buttonListViewAudioMid";
            this.buttonListViewAudioMid.Size = new System.Drawing.Size(745, 23);
            this.buttonListViewAudioMid.TabIndex = 11;
            // 
            // buttonListViewAudioIdle
            // 
            this.buttonListViewAudioIdle.Location = new System.Drawing.Point(127, 88);
            this.buttonListViewAudioIdle.Name = "buttonListViewAudioIdle";
            this.buttonListViewAudioIdle.Size = new System.Drawing.Size(745, 23);
            this.buttonListViewAudioIdle.TabIndex = 11;
            // 
            // buttonListViewAudioLow
            // 
            this.buttonListViewAudioLow.Location = new System.Drawing.Point(127, 126);
            this.buttonListViewAudioLow.Name = "buttonListViewAudioLow";
            this.buttonListViewAudioLow.Size = new System.Drawing.Size(745, 23);
            this.buttonListViewAudioLow.TabIndex = 11;
            // 
            // buttonOK
            // 
            this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOK.Location = new System.Drawing.Point(13, 257);
            this.buttonOK.Name = "buttonOK";
            this.buttonOK.Size = new System.Drawing.Size(75, 23);
            this.buttonOK.TabIndex = 12;
            this.buttonOK.Text = "OK";
            this.buttonOK.UseVisualStyleBackColor = true;
            // 
            // buttonCancel
            // 
            this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonCancel.Location = new System.Drawing.Point(797, 257);
            this.buttonCancel.Name = "buttonCancel";
            this.buttonCancel.Size = new System.Drawing.Size(75, 23);
            this.buttonCancel.TabIndex = 13;
            this.buttonCancel.Text = "Cancel";
            this.buttonCancel.UseVisualStyleBackColor = true;
            // 
            // openFileDialogH264
            // 
            this.openFileDialogH264.DefaultExt = "h264";
            this.openFileDialogH264.Filter = "Video|*.h264";
            // 
            // openFileDialogMp3
            // 
            this.openFileDialogMp3.DefaultExt = "mp3";
            this.openFileDialogMp3.Filter = "Audio|*.mp3";
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
            this.buttonDelete.Location = new System.Drawing.Point(405, 257);
            this.buttonDelete.Name = "buttonDelete";
            this.buttonDelete.Size = new System.Drawing.Size(75, 23);
            this.buttonDelete.TabIndex = 14;
            this.buttonDelete.Text = "Delete";
            this.buttonDelete.UseVisualStyleBackColor = true;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(11, 55);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(71, 12);
            this.label6.TabIndex = 15;
            this.label6.Text = "Video Idle:";
            // 
            // buttonVideoIdleAdd
            // 
            this.buttonVideoIdleAdd.Location = new System.Drawing.Point(89, 50);
            this.buttonVideoIdleAdd.Name = "buttonVideoIdleAdd";
            this.buttonVideoIdleAdd.Size = new System.Drawing.Size(32, 23);
            this.buttonVideoIdleAdd.TabIndex = 16;
            this.buttonVideoIdleAdd.Text = "Add";
            this.buttonVideoIdleAdd.UseVisualStyleBackColor = true;
            this.buttonVideoIdleAdd.Click += new System.EventHandler(this.buttonVideoIdleAdd_Click);
            // 
            // buttonListViewVideoIdle
            // 
            this.buttonListViewVideoIdle.Location = new System.Drawing.Point(127, 50);
            this.buttonListViewVideoIdle.Name = "buttonListViewVideoIdle";
            this.buttonListViewVideoIdle.Size = new System.Drawing.Size(745, 23);
            this.buttonListViewVideoIdle.TabIndex = 11;
            // 
            // Subaction
            // 
            this.AcceptButton = this.buttonOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonCancel;
            this.ClientSize = new System.Drawing.Size(884, 292);
            this.Controls.Add(this.buttonListViewVideoIdle);
            this.Controls.Add(this.buttonVideoIdleAdd);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.buttonDelete);
            this.Controls.Add(this.buttonCancel);
            this.Controls.Add(this.buttonOK);
            this.Controls.Add(this.buttonListViewAudioMid);
            this.Controls.Add(this.buttonListViewAudioLow);
            this.Controls.Add(this.buttonListViewAudioIdle);
            this.Controls.Add(this.buttonListViewAudioHigh);
            this.Controls.Add(this.buttonListViewVideo);
            this.Controls.Add(this.buttonAudioIdleAdd);
            this.Controls.Add(this.buttonAudioLowAdd);
            this.Controls.Add(this.buttonAudioHighAdd);
            this.Controls.Add(this.buttonAudioMidAdd);
            this.Controls.Add(this.buttonVideoAdd);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Name = "Subaction";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Subaction";
            this.contextMenuStrip.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Button buttonVideoAdd;
        private System.Windows.Forms.Button buttonAudioMidAdd;
        private System.Windows.Forms.Button buttonAudioHighAdd;
        private System.Windows.Forms.Button buttonAudioLowAdd;
        private System.Windows.Forms.Button buttonAudioIdleAdd;
        private ButtonListView buttonListViewVideo;
        private ButtonListView buttonListViewAudioHigh;
        private ButtonListView buttonListViewAudioMid;
        private ButtonListView buttonListViewAudioIdle;
        private ButtonListView buttonListViewAudioLow;
        private System.Windows.Forms.Button buttonOK;
        private System.Windows.Forms.Button buttonCancel;
        private System.Windows.Forms.OpenFileDialog openFileDialogH264;
        private System.Windows.Forms.OpenFileDialog openFileDialogMp3;
        private System.Windows.Forms.ToolTip toolTip;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem removeToolStripMenuItem;
        private System.Windows.Forms.Button buttonDelete;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Button buttonVideoIdleAdd;
        private ButtonListView buttonListViewVideoIdle;
    }
}