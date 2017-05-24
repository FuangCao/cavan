using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace JsonGenerator {
    public partial class Interaction : Form {
        private OpenDialogButton mButton;

        public Interaction() {
            InitializeComponent();
        }

        public StringBuilder generate(StringBuilder builder, String prefix, int index) {
            String localPrefix = prefix + "    ";

            builder.Append(prefix).AppendLine("{");

            builder.Append(localPrefix).AppendLine("\"main\":" + index + ",");
            builder.Append(localPrefix).Append("\"fov\":").Append(textBoxFov.Text).AppendLine(",");
            builder.Append(localPrefix).Append("\"name\":\"").Append(textBoxName.Text).AppendLine("\",");
            builder.Append(localPrefix).Append("\"video\":[\"").Append(textBoxVideo.Text).AppendLine("\"],");
            builder.Append(localPrefix).AppendLine("\"subaction\":[");

            int count = 0;

            foreach (Control control in buttonListViewSubaction.Controls) {
                Subaction subaction = (Subaction)(((OpenDialogButton)control).Dialog);
                if (count > 0) {
                    builder.AppendLine(",");
                }

                subaction.generate(builder, localPrefix + "    ", count++);
            }

            builder.AppendLine();
            builder.Append(localPrefix).AppendLine("]");

            builder.Append(prefix).Append('}');

            return builder;
        }

        public OpenDialogButton Button {
            get {
                return mButton;
            }

            set {
                mButton = value;
            }
        }

        private void textBoxVideo_Click(object sender, EventArgs e) {
            openFileDialogMp4.FileName = textBoxVideo.Text;
            if (openFileDialogMp4.ShowDialog() == DialogResult.OK) {
                textBoxVideo.Text = openFileDialogMp4.FileName;
            }
        }

        private void buttonSubactionAdd_Click(object sender, EventArgs e) {
            Subaction form = new Subaction();
            if (form.ShowDialog() == DialogResult.OK) {
                OpenDialogButton button = new OpenDialogButton(buttonListViewSubaction, form);
            }
        }

        private void removeToolStripMenuItem_Click(object sender, EventArgs e) {
            Control source = contextMenuStrip.SourceControl;
            if (source is OpenDialogButton) {
                ((OpenDialogButton)source).removeSelf();
            }
        }
    }
}
