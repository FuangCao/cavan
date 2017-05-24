using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace JsonGenerator {
    public partial class Generator : Form {
        public Generator() {
            InitializeComponent();
        }

        private void buttonBackAudioAdd_Click(object sender, EventArgs e) {
            if (openFileDialogAudio.ShowDialog() == DialogResult.OK) {
                OpenFileButton button = new OpenFileButton(buttonListViewBackAudio, openFileDialogAudio, toolTip);
                button.ContextMenuStrip = contextMenuStrip;
            }
        }

        private void buttonInteractionAdd_Click(object sender, EventArgs e) {
            Interaction form = new Interaction();
            if (form.ShowDialog() == DialogResult.OK) {
                OpenDialogButton button = new OpenDialogButton(buttonListViewInteraction, form);
                button.ContextMenuStrip = contextMenuStrip;
            }
        }

        private void addToolStripMenuItem_Click(object sender, EventArgs e) {
            Control source = contextMenuStrip.SourceControl;
            if (source is OpenFileButton) {
                ((OpenFileButton)source).removeSelf();
            } else if (source is OpenDialogButton) {
                ((OpenDialogButton)source).removeSelf();
            }
        }

        private void textBoxStartVideo_Click(object sender, EventArgs e) {
            openFileDialogVideoMp4.FileName = textBoxStartVideo.Text;
            if (openFileDialogVideoMp4.ShowDialog() == DialogResult.OK) {
                textBoxStartVideo.Text = openFileDialogVideoMp4.FileName;
            }
        }

        private void textBoxEndingVideo_Click(object sender, EventArgs e) {
            openFileDialogVideoMp4.FileName = textBoxEndingVideo.Text;
            if (openFileDialogVideoMp4.ShowDialog() == DialogResult.OK) {
                textBoxEndingVideo.Text = openFileDialogVideoMp4.FileName;
            }
        }

        private void buttonGenerate_Click(object sender, EventArgs e) {

        }
    }
}
