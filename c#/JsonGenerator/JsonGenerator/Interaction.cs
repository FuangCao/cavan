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
