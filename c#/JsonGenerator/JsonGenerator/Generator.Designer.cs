namespace JsonGenerator {
    partial class Generator {
        /// <summary>
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing) {
            if (disposing && (components != null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows 窗体设计器生成的代码

        /// <summary>
        /// 设计器支持所需的方法 - 不要修改
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent() {
            this.components = new System.ComponentModel.Container();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.radioButtonVideoType2D = new System.Windows.Forms.RadioButton();
            this.groupBoxVideoType = new System.Windows.Forms.GroupBox();
            this.radioButton2 = new System.Windows.Forms.RadioButton();
            this.radioButton1 = new System.Windows.Forms.RadioButton();
            this.textBoxDesc = new System.Windows.Forms.TextBox();
            this.textBoxStartAudio = new System.Windows.Forms.TextBox();
            this.textBoxEndingVideo = new System.Windows.Forms.TextBox();
            this.openFileDialogAudio = new System.Windows.Forms.OpenFileDialog();
            this.openFileDialogVideoMp4 = new System.Windows.Forms.OpenFileDialog();
            this.buttonBackAudioAdd = new System.Windows.Forms.Button();
            this.buttonInteractionAdd = new System.Windows.Forms.Button();
            this.openFileDialogVideoH264 = new System.Windows.Forms.OpenFileDialog();
            this.contextMenuStripEdit = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.addToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolTipPath = new System.Windows.Forms.ToolTip(this.components);
            this.controlListViewBackAudio = new JsonGenerator.ControlListView();
            this.groupBoxVideoType.SuspendLayout();
            this.contextMenuStripEdit.SuspendLayout();
            this.SuspendLayout();
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(10, 93);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(77, 12);
            this.label2.TabIndex = 1;
            this.label2.Text = "Description:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(10, 137);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(107, 12);
            this.label3.TabIndex = 2;
            this.label3.Text = "Background Audio:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(10, 181);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(77, 12);
            this.label4.TabIndex = 3;
            this.label4.Text = "Start Video:";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(10, 225);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(83, 12);
            this.label5.TabIndex = 4;
            this.label5.Text = "Ending Video:";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(10, 269);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(77, 12);
            this.label6.TabIndex = 5;
            this.label6.Text = "Interaction:";
            // 
            // radioButtonVideoType2D
            // 
            this.radioButtonVideoType2D.AutoSize = true;
            this.radioButtonVideoType2D.Location = new System.Drawing.Point(6, 21);
            this.radioButtonVideoType2D.Name = "radioButtonVideoType2D";
            this.radioButtonVideoType2D.Size = new System.Drawing.Size(35, 16);
            this.radioButtonVideoType2D.TabIndex = 6;
            this.radioButtonVideoType2D.TabStop = true;
            this.radioButtonVideoType2D.Text = "2D";
            this.radioButtonVideoType2D.UseVisualStyleBackColor = true;
            // 
            // groupBoxVideoType
            // 
            this.groupBoxVideoType.Controls.Add(this.radioButton2);
            this.groupBoxVideoType.Controls.Add(this.radioButton1);
            this.groupBoxVideoType.Controls.Add(this.radioButtonVideoType2D);
            this.groupBoxVideoType.Location = new System.Drawing.Point(12, 12);
            this.groupBoxVideoType.Name = "groupBoxVideoType";
            this.groupBoxVideoType.Size = new System.Drawing.Size(598, 53);
            this.groupBoxVideoType.TabIndex = 7;
            this.groupBoxVideoType.TabStop = false;
            this.groupBoxVideoType.Text = "Video Type";
            // 
            // radioButton2
            // 
            this.radioButton2.AutoSize = true;
            this.radioButton2.Location = new System.Drawing.Point(557, 21);
            this.radioButton2.Name = "radioButton2";
            this.radioButton2.Size = new System.Drawing.Size(35, 16);
            this.radioButton2.TabIndex = 8;
            this.radioButton2.TabStop = true;
            this.radioButton2.Text = "VR";
            this.radioButton2.UseVisualStyleBackColor = true;
            // 
            // radioButton1
            // 
            this.radioButton1.AutoSize = true;
            this.radioButton1.Location = new System.Drawing.Point(269, 21);
            this.radioButton1.Name = "radioButton1";
            this.radioButton1.Size = new System.Drawing.Size(59, 16);
            this.radioButton1.TabIndex = 7;
            this.radioButton1.TabStop = true;
            this.radioButton1.Text = "VR单目";
            this.radioButton1.UseVisualStyleBackColor = true;
            // 
            // textBoxDesc
            // 
            this.textBoxDesc.Location = new System.Drawing.Point(93, 93);
            this.textBoxDesc.Name = "textBoxDesc";
            this.textBoxDesc.Size = new System.Drawing.Size(510, 21);
            this.textBoxDesc.TabIndex = 8;
            // 
            // textBoxStartAudio
            // 
            this.textBoxStartAudio.Location = new System.Drawing.Point(129, 171);
            this.textBoxStartAudio.Name = "textBoxStartAudio";
            this.textBoxStartAudio.Size = new System.Drawing.Size(474, 21);
            this.textBoxStartAudio.TabIndex = 10;
            // 
            // textBoxEndingVideo
            // 
            this.textBoxEndingVideo.Location = new System.Drawing.Point(129, 225);
            this.textBoxEndingVideo.Name = "textBoxEndingVideo";
            this.textBoxEndingVideo.Size = new System.Drawing.Size(474, 21);
            this.textBoxEndingVideo.TabIndex = 11;
            // 
            // openFileDialogAudio
            // 
            this.openFileDialogAudio.DefaultExt = "mp3";
            this.openFileDialogAudio.Filter = "audio|*.mp3";
            // 
            // openFileDialogVideoMp4
            // 
            this.openFileDialogVideoMp4.DefaultExt = "mp4";
            this.openFileDialogVideoMp4.Filter = "video|*.mp4";
            // 
            // buttonBackAudioAdd
            // 
            this.buttonBackAudioAdd.Location = new System.Drawing.Point(129, 137);
            this.buttonBackAudioAdd.Name = "buttonBackAudioAdd";
            this.buttonBackAudioAdd.Size = new System.Drawing.Size(36, 23);
            this.buttonBackAudioAdd.TabIndex = 12;
            this.buttonBackAudioAdd.Text = "Add";
            this.buttonBackAudioAdd.UseVisualStyleBackColor = true;
            this.buttonBackAudioAdd.Click += new System.EventHandler(this.buttonBackAudioAdd_Click);
            // 
            // buttonInteractionAdd
            // 
            this.buttonInteractionAdd.Location = new System.Drawing.Point(112, 264);
            this.buttonInteractionAdd.Name = "buttonInteractionAdd";
            this.buttonInteractionAdd.Size = new System.Drawing.Size(32, 23);
            this.buttonInteractionAdd.TabIndex = 13;
            this.buttonInteractionAdd.Text = "Add";
            this.buttonInteractionAdd.UseVisualStyleBackColor = true;
            this.buttonInteractionAdd.Click += new System.EventHandler(this.buttonInteractionAdd_Click);
            // 
            // openFileDialogVideoH264
            // 
            this.openFileDialogVideoH264.DefaultExt = "h264";
            this.openFileDialogVideoH264.Filter = "video|*.h264";
            // 
            // contextMenuStripEdit
            // 
            this.contextMenuStripEdit.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addToolStripMenuItem});
            this.contextMenuStripEdit.Name = "contextMenuStripEdit";
            this.contextMenuStripEdit.Size = new System.Drawing.Size(124, 26);
            // 
            // addToolStripMenuItem
            // 
            this.addToolStripMenuItem.Name = "addToolStripMenuItem";
            this.addToolStripMenuItem.Size = new System.Drawing.Size(123, 22);
            this.addToolStripMenuItem.Text = "Remove";
            this.addToolStripMenuItem.Click += new System.EventHandler(this.addToolStripMenuItem_Click);
            // 
            // controlListViewBackAudio
            // 
            this.controlListViewBackAudio.Location = new System.Drawing.Point(172, 137);
            this.controlListViewBackAudio.Name = "controlListViewBackAudio";
            this.controlListViewBackAudio.Size = new System.Drawing.Size(431, 23);
            this.controlListViewBackAudio.TabIndex = 14;
            // 
            // Generator
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(622, 511);
            this.Controls.Add(this.controlListViewBackAudio);
            this.Controls.Add(this.buttonInteractionAdd);
            this.Controls.Add(this.buttonBackAudioAdd);
            this.Controls.Add(this.textBoxEndingVideo);
            this.Controls.Add(this.textBoxStartAudio);
            this.Controls.Add(this.textBoxDesc);
            this.Controls.Add(this.groupBoxVideoType);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Name = "Generator";
            this.Text = "JsonGenerator";
            this.groupBoxVideoType.ResumeLayout(false);
            this.groupBoxVideoType.PerformLayout();
            this.contextMenuStripEdit.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.RadioButton radioButtonVideoType2D;
        private System.Windows.Forms.GroupBox groupBoxVideoType;
        private System.Windows.Forms.RadioButton radioButton2;
        private System.Windows.Forms.RadioButton radioButton1;
        private System.Windows.Forms.TextBox textBoxDesc;
        private System.Windows.Forms.TextBox textBoxStartAudio;
        private System.Windows.Forms.TextBox textBoxEndingVideo;
        private System.Windows.Forms.OpenFileDialog openFileDialogAudio;
        private System.Windows.Forms.OpenFileDialog openFileDialogVideoMp4;
        private System.Windows.Forms.Button buttonBackAudioAdd;
        private System.Windows.Forms.Button buttonInteractionAdd;
        private System.Windows.Forms.OpenFileDialog openFileDialogVideoH264;
        private ControlListView controlListViewBackAudio;
        private System.Windows.Forms.ContextMenuStrip contextMenuStripEdit;
        private System.Windows.Forms.ToolStripMenuItem addToolStripMenuItem;
        private System.Windows.Forms.ToolTip toolTipPath;
    }
}

