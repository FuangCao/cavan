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
            this.radioButtonVideoTypeVr = new System.Windows.Forms.RadioButton();
            this.radioButtonVideoTypeVrSingle = new System.Windows.Forms.RadioButton();
            this.textBoxDesc = new System.Windows.Forms.TextBox();
            this.textBoxStartVideo = new System.Windows.Forms.TextBox();
            this.textBoxEndingVideo = new System.Windows.Forms.TextBox();
            this.openFileDialogAudio = new System.Windows.Forms.OpenFileDialog();
            this.openFileDialogVideoMp4 = new System.Windows.Forms.OpenFileDialog();
            this.buttonBackAudioAdd = new System.Windows.Forms.Button();
            this.buttonInteractionAdd = new System.Windows.Forms.Button();
            this.openFileDialogVideoH264 = new System.Windows.Forms.OpenFileDialog();
            this.contextMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.addToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.buttonGenerate = new System.Windows.Forms.Button();
            this.buttonClose = new System.Windows.Forms.Button();
            this.saveFileDialogJson = new System.Windows.Forms.SaveFileDialog();
            this.buttonListViewInteraction = new JsonGenerator.ButtonListView();
            this.buttonListViewBackAudio = new JsonGenerator.ButtonListView();
            this.groupBoxVideoType.SuspendLayout();
            this.contextMenuStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 85);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(77, 12);
            this.label2.TabIndex = 1;
            this.label2.Text = "Description:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 123);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(107, 12);
            this.label3.TabIndex = 2;
            this.label3.Text = "Background Audio:";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(12, 161);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(77, 12);
            this.label4.TabIndex = 3;
            this.label4.Text = "Start Video:";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(12, 198);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(83, 12);
            this.label5.TabIndex = 4;
            this.label5.Text = "Ending Video:";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(12, 236);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(77, 12);
            this.label6.TabIndex = 5;
            this.label6.Text = "Interaction:";
            // 
            // radioButtonVideoType2D
            // 
            this.radioButtonVideoType2D.AutoSize = true;
            this.radioButtonVideoType2D.Location = new System.Drawing.Point(14, 21);
            this.radioButtonVideoType2D.Name = "radioButtonVideoType2D";
            this.radioButtonVideoType2D.Size = new System.Drawing.Size(35, 16);
            this.radioButtonVideoType2D.TabIndex = 6;
            this.radioButtonVideoType2D.TabStop = true;
            this.radioButtonVideoType2D.Text = "2D";
            this.radioButtonVideoType2D.UseVisualStyleBackColor = true;
            // 
            // groupBoxVideoType
            // 
            this.groupBoxVideoType.Controls.Add(this.radioButtonVideoTypeVr);
            this.groupBoxVideoType.Controls.Add(this.radioButtonVideoTypeVrSingle);
            this.groupBoxVideoType.Controls.Add(this.radioButtonVideoType2D);
            this.groupBoxVideoType.Location = new System.Drawing.Point(12, 12);
            this.groupBoxVideoType.Name = "groupBoxVideoType";
            this.groupBoxVideoType.Size = new System.Drawing.Size(860, 53);
            this.groupBoxVideoType.TabIndex = 7;
            this.groupBoxVideoType.TabStop = false;
            this.groupBoxVideoType.Text = "Video Type";
            // 
            // radioButtonVideoTypeVr
            // 
            this.radioButtonVideoTypeVr.AutoSize = true;
            this.radioButtonVideoTypeVr.Location = new System.Drawing.Point(808, 21);
            this.radioButtonVideoTypeVr.Name = "radioButtonVideoTypeVr";
            this.radioButtonVideoTypeVr.Size = new System.Drawing.Size(35, 16);
            this.radioButtonVideoTypeVr.TabIndex = 8;
            this.radioButtonVideoTypeVr.TabStop = true;
            this.radioButtonVideoTypeVr.Text = "VR";
            this.radioButtonVideoTypeVr.UseVisualStyleBackColor = true;
            // 
            // radioButtonVideoTypeVrSingle
            // 
            this.radioButtonVideoTypeVrSingle.AutoSize = true;
            this.radioButtonVideoTypeVrSingle.Location = new System.Drawing.Point(399, 21);
            this.radioButtonVideoTypeVrSingle.Name = "radioButtonVideoTypeVrSingle";
            this.radioButtonVideoTypeVrSingle.Size = new System.Drawing.Size(59, 16);
            this.radioButtonVideoTypeVrSingle.TabIndex = 7;
            this.radioButtonVideoTypeVrSingle.TabStop = true;
            this.radioButtonVideoTypeVrSingle.Text = "VR单目";
            this.radioButtonVideoTypeVrSingle.UseVisualStyleBackColor = true;
            // 
            // textBoxDesc
            // 
            this.textBoxDesc.Location = new System.Drawing.Point(118, 81);
            this.textBoxDesc.Name = "textBoxDesc";
            this.textBoxDesc.Size = new System.Drawing.Size(754, 21);
            this.textBoxDesc.TabIndex = 8;
            // 
            // textBoxStartVideo
            // 
            this.textBoxStartVideo.Location = new System.Drawing.Point(125, 157);
            this.textBoxStartVideo.Name = "textBoxStartVideo";
            this.textBoxStartVideo.ReadOnly = true;
            this.textBoxStartVideo.Size = new System.Drawing.Size(747, 21);
            this.textBoxStartVideo.TabIndex = 10;
            this.textBoxStartVideo.Click += new System.EventHandler(this.textBoxStartVideo_Click);
            // 
            // textBoxEndingVideo
            // 
            this.textBoxEndingVideo.Location = new System.Drawing.Point(125, 194);
            this.textBoxEndingVideo.Name = "textBoxEndingVideo";
            this.textBoxEndingVideo.ReadOnly = true;
            this.textBoxEndingVideo.Size = new System.Drawing.Size(747, 21);
            this.textBoxEndingVideo.TabIndex = 11;
            this.textBoxEndingVideo.Click += new System.EventHandler(this.textBoxEndingVideo_Click);
            // 
            // openFileDialogAudio
            // 
            this.openFileDialogAudio.DefaultExt = "mp3";
            this.openFileDialogAudio.Filter = "Audio|*.mp3";
            // 
            // openFileDialogVideoMp4
            // 
            this.openFileDialogVideoMp4.DefaultExt = "mp4";
            this.openFileDialogVideoMp4.Filter = "Video|*.mp4";
            // 
            // buttonBackAudioAdd
            // 
            this.buttonBackAudioAdd.Location = new System.Drawing.Point(125, 118);
            this.buttonBackAudioAdd.Name = "buttonBackAudioAdd";
            this.buttonBackAudioAdd.Size = new System.Drawing.Size(36, 23);
            this.buttonBackAudioAdd.TabIndex = 12;
            this.buttonBackAudioAdd.Text = "Add";
            this.buttonBackAudioAdd.UseVisualStyleBackColor = true;
            this.buttonBackAudioAdd.Click += new System.EventHandler(this.buttonBackAudioAdd_Click);
            // 
            // buttonInteractionAdd
            // 
            this.buttonInteractionAdd.Location = new System.Drawing.Point(125, 231);
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
            this.openFileDialogVideoH264.Filter = "Video|*.h264";
            // 
            // contextMenuStrip
            // 
            this.contextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addToolStripMenuItem});
            this.contextMenuStrip.Name = "contextMenuStripEdit";
            this.contextMenuStrip.Size = new System.Drawing.Size(114, 26);
            // 
            // addToolStripMenuItem
            // 
            this.addToolStripMenuItem.Name = "addToolStripMenuItem";
            this.addToolStripMenuItem.Size = new System.Drawing.Size(113, 22);
            this.addToolStripMenuItem.Text = "Delete";
            this.addToolStripMenuItem.Click += new System.EventHandler(this.addToolStripMenuItem_Click);
            // 
            // buttonGenerate
            // 
            this.buttonGenerate.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonGenerate.Location = new System.Drawing.Point(12, 286);
            this.buttonGenerate.Name = "buttonGenerate";
            this.buttonGenerate.Size = new System.Drawing.Size(75, 23);
            this.buttonGenerate.TabIndex = 15;
            this.buttonGenerate.Text = "Generate";
            this.buttonGenerate.UseVisualStyleBackColor = true;
            this.buttonGenerate.Click += new System.EventHandler(this.buttonGenerate_Click);
            // 
            // buttonClose
            // 
            this.buttonClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.buttonClose.Location = new System.Drawing.Point(797, 286);
            this.buttonClose.Name = "buttonClose";
            this.buttonClose.Size = new System.Drawing.Size(75, 23);
            this.buttonClose.TabIndex = 17;
            this.buttonClose.Text = "Close";
            this.buttonClose.UseVisualStyleBackColor = true;
            this.buttonClose.Click += new System.EventHandler(this.buttonClose_Click);
            // 
            // saveFileDialogJson
            // 
            this.saveFileDialogJson.DefaultExt = "json";
            this.saveFileDialogJson.FileName = "VideoGame";
            this.saveFileDialogJson.Filter = "Json|*.json";
            // 
            // buttonListViewInteraction
            // 
            this.buttonListViewInteraction.Location = new System.Drawing.Point(167, 231);
            this.buttonListViewInteraction.Name = "buttonListViewInteraction";
            this.buttonListViewInteraction.Size = new System.Drawing.Size(705, 23);
            this.buttonListViewInteraction.TabIndex = 16;
            // 
            // buttonListViewBackAudio
            // 
            this.buttonListViewBackAudio.Location = new System.Drawing.Point(167, 118);
            this.buttonListViewBackAudio.Name = "buttonListViewBackAudio";
            this.buttonListViewBackAudio.Size = new System.Drawing.Size(705, 23);
            this.buttonListViewBackAudio.TabIndex = 14;
            // 
            // Generator
            // 
            this.AcceptButton = this.buttonGenerate;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.buttonClose;
            this.ClientSize = new System.Drawing.Size(884, 321);
            this.Controls.Add(this.buttonClose);
            this.Controls.Add(this.buttonListViewInteraction);
            this.Controls.Add(this.buttonGenerate);
            this.Controls.Add(this.buttonListViewBackAudio);
            this.Controls.Add(this.buttonInteractionAdd);
            this.Controls.Add(this.buttonBackAudioAdd);
            this.Controls.Add(this.textBoxEndingVideo);
            this.Controls.Add(this.textBoxStartVideo);
            this.Controls.Add(this.textBoxDesc);
            this.Controls.Add(this.groupBoxVideoType);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Name = "Generator";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "JsonGenerator";
            this.groupBoxVideoType.ResumeLayout(false);
            this.groupBoxVideoType.PerformLayout();
            this.contextMenuStrip.ResumeLayout(false);
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
        private System.Windows.Forms.RadioButton radioButtonVideoTypeVr;
        private System.Windows.Forms.RadioButton radioButtonVideoTypeVrSingle;
        private System.Windows.Forms.TextBox textBoxDesc;
        private System.Windows.Forms.TextBox textBoxStartVideo;
        private System.Windows.Forms.TextBox textBoxEndingVideo;
        private System.Windows.Forms.OpenFileDialog openFileDialogAudio;
        private System.Windows.Forms.OpenFileDialog openFileDialogVideoMp4;
        private System.Windows.Forms.Button buttonBackAudioAdd;
        private System.Windows.Forms.Button buttonInteractionAdd;
        private System.Windows.Forms.OpenFileDialog openFileDialogVideoH264;
        private ButtonListView buttonListViewBackAudio;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem addToolStripMenuItem;
        private System.Windows.Forms.ToolTip toolTip;
        private System.Windows.Forms.Button buttonGenerate;
        private ButtonListView buttonListViewInteraction;
        private System.Windows.Forms.Button buttonClose;
        private System.Windows.Forms.SaveFileDialog saveFileDialogJson;
    }
}

