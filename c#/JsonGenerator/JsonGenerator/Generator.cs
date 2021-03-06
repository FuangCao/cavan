﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace JsonGenerator {
    public partial class Generator : Form {
        public Generator() {
            InitializeComponent();
        }

        public StringBuilder generate(StringBuilder builder) {
            String subPrefix = "    ";

            builder.Append('{').AppendLine();
            builder.Append(subPrefix).AppendLine("\"version\":1,");
            builder.Append(subPrefix).Append("\"sourcetype\":\"").Append(getSourceType()).AppendLine("\",");
            builder.Append(subPrefix).Append("\"desc\":\"").Append(textBoxDesc.Text).AppendLine("\",");
            buttonListViewBackAudio.generate(builder, subPrefix, "audioback").Append(',').AppendLine();
            builder.Append(subPrefix).Append("\"startvideo\":\"").Append(Path.GetFileName(textBoxStartVideo.Text)).AppendLine("\",");
            builder.Append(subPrefix).Append("\"endingvideo\":\"").Append(Path.GetFileName(textBoxEndingVideo.Text)).AppendLine("\",");
            buttonListViewInteraction.generate(builder, subPrefix, "action").AppendLine();
            builder.Append('}');

            return builder;
        }

        public bool generate(String pathname) {
            StreamWriter writer = null;

            try {
                writer = File.CreateText(pathname);
                StringBuilder builder = new StringBuilder();
                generate(builder);
                writer.Write(builder.ToString());
                return true;
            } catch (Exception e) {
                MessageBox.Show(e.ToString());
            } finally {
                if (writer != null) {
                    writer.Close();
                }
            }

            return false;
        }

        public int getSourceType() {
            if (radioButtonVideoType2D.Checked) {
                return 2;
            }

            if (radioButtonVideoTypeVrSingle.Checked) {
                return 1;
            }

            if (radioButtonVideoTypeVr.Checked) {
                return 3;
            }

            return 0;
        }

        private void buttonBackAudioAdd_Click(object sender, EventArgs e) {
            buttonListViewBackAudio.addOpenFileButtons(openFileDialogMp3, contextMenuStrip, toolTip);
        }

        private void buttonInteractionAdd_Click(object sender, EventArgs e) {
            Interaction form = new Interaction();
            if (form.ShowDialog(false) == DialogResult.OK) {
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
            openFileDialogMp4.FileName = textBoxStartVideo.Text;
            if (openFileDialogMp4.ShowDialog() == DialogResult.OK) {
                textBoxStartVideo.Text = openFileDialogMp4.FileName;
            }
        }

        private void textBoxEndingVideo_Click(object sender, EventArgs e) {
            openFileDialogMp4.FileName = textBoxEndingVideo.Text;
            if (openFileDialogMp4.ShowDialog() == DialogResult.OK) {
                textBoxEndingVideo.Text = openFileDialogMp4.FileName;
            }
        }

        private void buttonGenerate_Click(object sender, EventArgs e) {
            if (saveFileDialogJson.ShowDialog() == DialogResult.OK) {
                if (generate(saveFileDialogJson.FileName)) {
                    MessageBox.Show("Successfully generated!");
                }
            }
        }

        private void buttonClose_Click(object sender, EventArgs e) {
            Close();
        }
    }
}
