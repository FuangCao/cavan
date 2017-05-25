using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace JsonGenerator {
    public class OpenDialogButton : GenerableButton {
        private ButtonListView mListView;
        private GenerableForm mDialog;
        private int mIndex;

        public OpenDialogButton(ButtonListView listView, GenerableForm form) {
            mListView = listView;
            mDialog = form;
            form.GenerableButton = this;
            Index = listView.Controls.Count;
            Click += OpenDialogButton_Click;
            listView.addButton(this);
        }

        public Form Dialog {
            get {
                return mDialog;
            }
        }

        public int Index {
            get {
                return mIndex;
            }

            set {
                mIndex = value;
                Text = Convert.ToString(value);
            }
        }

        private void OpenDialogButton_Click(object sender, EventArgs e) {
            if (mDialog.ShowDialog(true) == DialogResult.No) {
                removeSelf();
            }
        }

        public void removeSelf() {
            mListView.removeButton(this);

            int index = 0;

            foreach (Control control in mListView.Controls) {
                ((OpenDialogButton)control).Index = index++;
            }
        }

        public override StringBuilder generate(StringBuilder builder, string prefix, int index) {
            return mDialog.generate(builder, prefix, index);
        }
    }
}
