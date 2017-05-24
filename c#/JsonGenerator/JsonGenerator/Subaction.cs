using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace JsonGenerator {
    public partial class Subaction : Form {
        public Subaction() {
            InitializeComponent();
        }

        public StringBuilder generate(StringBuilder builder, String prefix, int index) {
            String localPrefix = prefix + "    ";

            builder.AppendLine(prefix + '{');

            builder.AppendLine(localPrefix + "\"mirror\":" + index);
            buttonListViewVideo.generate(builder, localPrefix, "videofile").AppendLine(",");
            buttonListViewAudioIdle.generate(builder, localPrefix, "audioidle").AppendLine(",");
            buttonListViewAudioLow.generate(builder, localPrefix, "audiolow").AppendLine(",");
            buttonListViewAudioMid.generate(builder, localPrefix, "audiomid").AppendLine(",");
            buttonListViewAudioHigh.generate(builder, localPrefix, "audiohigh").AppendLine();

            builder.Append(prefix).Append('}');

            return builder;
        }

        private void buttonVideoAdd_Click(object sender, EventArgs e) {
            if (openFileDialogH264.ShowDialog() == DialogResult.OK) {
                OpenFileButton button = new OpenFileButton(buttonListViewVideo, openFileDialogH264, toolTip);
                button.ContextMenuStrip = contextMenuStrip;
            }
        }

        private void buttonAudioIdleAdd_Click(object sender, EventArgs e) {
            if (openFileDialogMp3.ShowDialog() == DialogResult.OK) {
                OpenFileButton button = new OpenFileButton(buttonListViewAudioIdle, openFileDialogMp3, toolTip);
                button.ContextMenuStrip = contextMenuStrip;
            }
        }

        private void buttonAudioLowAdd_Click(object sender, EventArgs e) {
            if (openFileDialogMp3.ShowDialog() == DialogResult.OK) {
                OpenFileButton button = new OpenFileButton(buttonListViewAudioLow, openFileDialogMp3, toolTip);
                button.ContextMenuStrip = contextMenuStrip;
            }
        }

        private void buttonAudioMidAdd_Click(object sender, EventArgs e) {
            if (openFileDialogMp3.ShowDialog() == DialogResult.OK) {
                OpenFileButton button = new OpenFileButton(buttonListViewAudioMid, openFileDialogMp3, toolTip);
                button.ContextMenuStrip = contextMenuStrip;
            }
        }

        private void buttonAudioHighAdd_Click(object sender, EventArgs e) {
            if (openFileDialogMp3.ShowDialog() == DialogResult.OK) {
                OpenFileButton button = new OpenFileButton(buttonListViewAudioHigh, openFileDialogMp3, toolTip);
                button.ContextMenuStrip = contextMenuStrip;
            }
        }

        private void removeToolStripMenuItem_Click(object sender, EventArgs e) {
            Control source = contextMenuStrip.SourceControl;
            if (source is OpenFileButton) {
                ((OpenFileButton)source).removeSelf();
            }
        }
    }
}
