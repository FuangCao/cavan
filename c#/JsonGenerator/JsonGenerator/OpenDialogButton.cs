using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace JsonGenerator {
    public class OpenDialogButton : Button {
        private ButtonListView mListView;
        private Form mDialog;
        private int mIndex;

        public OpenDialogButton(ButtonListView listView, Form form) {
            mListView = listView;
            mDialog = form;
            Index = listView.Controls.Count;
            Click += OpenDialogButton_Click;
            listView.addButton(this);
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
            mDialog.ShowDialog();
        }

        public void removeSelf() {
            mListView.removeButton(this);

            int index = 0;

            foreach (Control control in mListView.Controls) {
                ((OpenDialogButton)control).Index = index++;
            }
        }
    }
}
