using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace JsonGenerator {
    class OpenFileButton : GenerableButton {
        private ButtonListView mListView;
        private OpenFileDialog mDialog;
        private FileInfo mFileInfo;
        private ToolTip mToolTip;

        public OpenFileButton(ButtonListView listView, OpenFileDialog dialog, String filename, ContextMenuStrip menu, ToolTip tooltip) {
            mListView = listView;
            mDialog = dialog;
            mToolTip = tooltip;
            FullName = filename;
            ContextMenuStrip = menu;
            Click += OpenFileButton_Click;
        }

        private void OpenFileButton_Click(object sender, EventArgs e) {
            if (mDialog == null) {
                return;
            }

            mDialog.Multiselect = false;

            if (mFileInfo != null) {
                mDialog.FileName = mFileInfo.FullName;
            } else {
                mDialog.FileName = null;
            }

            if (mDialog.ShowDialog() == DialogResult.OK) {
                FullName = mDialog.FileName;
                mListView.updateButtonBounds();
            }
        }

        public FileInfo FileInfo {
            get {
                return mFileInfo;
            }

            set {
                mFileInfo = value;
                Text = mFileInfo.Name;

                mToolTip.SetToolTip(this, mFileInfo.FullName);
            }
        }

        public String FullName {
            get {
                if (mFileInfo == null) {
                    return null;
                }

                return mFileInfo.FullName;
            }

            set {
                if (value != null) {
                    FileInfo = new FileInfo(value);
                }
            }
        }

        public String FileName {
            get {
                if (mFileInfo == null) {
                    return null;
                }

                return mFileInfo.Name;
            }
        }

        public ToolTip ToolTip {
            get {
                return mToolTip;
            }

            set {
                mToolTip = value;
            }
        }

        public void removeSelf() {
            mListView.removeButton(this);
        }

        public override StringBuilder generate(StringBuilder builder, string prefix, int index) {
            return builder.Append('"').Append(mFileInfo.Name).Append('"');
        }
    }
}
