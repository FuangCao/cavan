using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace JsonGenerator {
    public partial class Subaction : GenerableForm {
        public Subaction() {
            InitializeComponent();
            buttonDelete.Visible = (GenerableButton != null);
        }

        public override StringBuilder generate(StringBuilder builder, String prefix, int index) {
            String subPrefix = prefix + "    ";

            builder.Append(prefix).Append('{').AppendLine();

            builder.Append(subPrefix).Append("\"minor\":").Append(index).Append(',').AppendLine();
            ButtonListView.generate(builder, subPrefix, "videofile", buttonListViewVideo, buttonListViewVideoIdle).Append(',').AppendLine();
            buttonListViewAudioIdle.generate(builder, subPrefix, "audioidle").Append(',').AppendLine();
            buttonListViewAudioLow.generate(builder, subPrefix, "audiolow").Append(',').AppendLine();
            buttonListViewAudioMid.generate(builder, subPrefix, "audiomid").Append(',').AppendLine();
            buttonListViewAudioHigh.generate(builder, subPrefix, "audiohigh").AppendLine();

            builder.Append(prefix).Append('}');

            return builder;
        }

        public override DialogResult ShowDialog(bool deleteEn) {
            buttonDelete.Visible = deleteEn;
            return ShowDialog();
        }

        private void buttonVideoAdd_Click(object sender, EventArgs e) {
            buttonListViewVideo.addOpenFileButtons(openFileDialogH264, contextMenuStrip, toolTip);
        }

        private void buttonVideoIdleAdd_Click(object sender, EventArgs e) {
            buttonListViewVideoIdle.addOpenFileButtons(openFileDialogH264, contextMenuStrip, toolTip);
        }

        private void buttonAudioIdleAdd_Click(object sender, EventArgs e) {
            buttonListViewAudioIdle.addOpenFileButtons(openFileDialogMp3, contextMenuStrip, toolTip);
        }

        private void buttonAudioLowAdd_Click(object sender, EventArgs e) {
            buttonListViewAudioLow.addOpenFileButtons(openFileDialogMp3, contextMenuStrip, toolTip);
        }

        private void buttonAudioMidAdd_Click(object sender, EventArgs e) {
            buttonListViewAudioMid.addOpenFileButtons(openFileDialogMp3, contextMenuStrip, toolTip);
        }

        private void buttonAudioHighAdd_Click(object sender, EventArgs e) {
            buttonListViewAudioHigh.addOpenFileButtons(openFileDialogMp3, contextMenuStrip, toolTip);
        }

        private void removeToolStripMenuItem_Click(object sender, EventArgs e) {
            Control source = contextMenuStrip.SourceControl;
            if (source is OpenFileButton) {
                ((OpenFileButton)source).removeSelf();
            }
        }
    }
}
