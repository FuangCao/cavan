using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Windows.Forms;

namespace JsonGenerator {
    class FileButton : Button {
        private OpenFileDialog mDialog;
        private FileInfo mFileInfo;
        private ToolTip mToolTip;

        public FileButton() {
            Click += FileButton_Click;
        }

        private void FileButton_Click(object sender, EventArgs e) {
            if (mDialog == null) {
                return;
            }

            if (mFileInfo != null) {
                mDialog.FileName = mFileInfo.FullName;
            } else {
                mDialog.FileName = null;
            }
            
            if (mDialog.ShowDialog() == DialogResult.OK) {
                FullName = mDialog.FileName;
            }
        }

        public OpenFileDialog Dialog {
            get {
                return mDialog;
            }

            set {
                mDialog = value;
                FullName = mDialog.FileName;
            }
        }

        public FileInfo FileInfo {
            get {
                return mFileInfo;
            }

            set {
                mFileInfo = value;
                Text = mFileInfo.Name;

                if (mToolTip != null) {
                    mToolTip.SetToolTip(this, mFileInfo.FullName);
                }
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
    }
}
