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

        private void addBackAudio() {
        }

        private void buttonBackAudioAdd_Click(object sender, EventArgs e) {
            if (openFileDialogAudio.ShowDialog() == DialogResult.OK) {
                FileButton button = new FileButton();
                button.ToolTip = toolTipPath;
                button.ContextMenuStrip = contextMenuStripEdit;
                button.Dialog = openFileDialogAudio;
                controlListViewBackAudio.addControl(button);
            }
        }

        private void buttonInteractionAdd_Click(object sender, EventArgs e) {

        }

        private void addToolStripMenuItem_Click(object sender, EventArgs e) {
            Control control = contextMenuStripEdit.SourceControl;
            Control parent = control.Parent;
            if (parent is ControlListView) {
                ((ControlListView)parent).removeControl(control);
            }
        }
    }
}
