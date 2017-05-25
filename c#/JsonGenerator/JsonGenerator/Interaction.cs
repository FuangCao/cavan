using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace JsonGenerator {
    public partial class Interaction : GenerableForm {
        public Interaction() {
            InitializeComponent();
            buttonDelete.Visible = (GenerableButton != null);
        }

        public override StringBuilder generate(StringBuilder builder, String prefix, int index) {
            String localPrefix = prefix + "    ";

            builder.Append(prefix).AppendLine("{");

            builder.Append(localPrefix).Append("\"main\":").Append(index).Append(',').AppendLine();
            builder.Append(localPrefix).Append("\"fov\":").Append(textBoxFov.Text).Append(',').AppendLine();
            builder.Append(localPrefix).Append("\"name\":\"").Append(textBoxName.Text).AppendLine("\",");
            builder.Append(localPrefix).Append("\"video\":[\"").Append(Path.GetFileName(textBoxVideo.Text)).AppendLine("\"],");
            buttonListViewSubaction.generate(builder, localPrefix, "subaction").AppendLine();

            builder.Append(prefix).Append('}');

            return builder;
        }

        public override DialogResult ShowDialog(bool deleteEn) {
            buttonDelete.Visible = deleteEn;
            return ShowDialog();
        }

        private void textBoxVideo_Click(object sender, EventArgs e) {
            openFileDialogMp4.FileName = textBoxVideo.Text;
            if (openFileDialogMp4.ShowDialog() == DialogResult.OK) {
                textBoxVideo.Text = openFileDialogMp4.FileName;
            }
        }

        private void buttonSubactionAdd_Click(object sender, EventArgs e) {
            Subaction form = new Subaction();
            if (form.ShowDialog(false) == DialogResult.OK) {
                OpenDialogButton button = new OpenDialogButton(buttonListViewSubaction, form);
                button.ContextMenuStrip = contextMenuStrip;
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
