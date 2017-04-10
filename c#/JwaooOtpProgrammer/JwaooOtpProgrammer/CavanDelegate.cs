using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace JwaooOtpProgrammer {
    public class CavanDelegate {

        private delegate String GetTextDelegate(Control control);
        private static GetTextDelegate mGetTextDelegate = new GetTextDelegate(getText);

        private delegate void SetTextDelegate(Control control, String text);
        private static SetTextDelegate mSetTextDelegate = new SetTextDelegate(setText);

        private delegate void SetForeColorDelegate(Control control, Color color);
        private static SetForeColorDelegate mSetForeColorDelegate = new SetForeColorDelegate(setForeColor);

        private delegate void SetBackColorDelegate(Control control, Color color);
        private static SetBackColorDelegate mSetBackColorDelegate = new SetBackColorDelegate(setBackColor);

        private delegate bool IsEnabledDelegate(Control control);
        private static IsEnabledDelegate mIsEnabledDelegate = new IsEnabledDelegate(isEnabled);

        private delegate void SetEnableDelegate(Control control, bool enable);
        private static SetEnableDelegate mSetEnableDelegate = new SetEnableDelegate(setEnable);

        private delegate void AppendTextDelegate(TextBoxBase control, String text);
        private static AppendTextDelegate mAppendTextDelegate = new AppendTextDelegate(appendText);

        private delegate void ClearTextDelegate(TextBoxBase control);
        private static ClearTextDelegate mClearTextDelegate = new ClearTextDelegate(clearText);

        public static void clearText(TextBoxBase control) {
            if (control.InvokeRequired) {
                control.Invoke(mClearTextDelegate, new object[] { control });
            } else {
                control.Clear();
            }
        }

        public static void appendText(TextBoxBase control, String text) {
            if (control.InvokeRequired) {
                control.Invoke(mAppendTextDelegate, new object[] { control, text });
            } else {
                control.AppendText(text);
            }
        }

        public static bool isEnabled(Control control) {
            if (control.InvokeRequired) {
                return (bool)control.Invoke(mIsEnabledDelegate, new object[] { control });
            } else {
                return control.Enabled;
            }
        }

        public static void setEnable(Control control, bool enable) {
            if (control.InvokeRequired) {
                control.Invoke(mSetEnableDelegate, new object[] { control, enable });
            } else {
                control.Enabled = enable;
            }
        }

        public static void setBackColor(Control control, Color color) {
            if (control.InvokeRequired) {
                control.Invoke(mSetBackColorDelegate, new object[] { control, color });
            } else {
                control.BackColor = color;
            }
        }

        public static void setForeColor(Control control, Color color) {
            if (control.InvokeRequired) {
                control.Invoke(mSetForeColorDelegate, new object[] { control, color });
            } else {
                control.ForeColor = color;
            }
        }

        public static String getText(Control control) {
            if (control.InvokeRequired) {
                return (String)control.Invoke(mGetTextDelegate, new object[] { control });
            } else {
                return control.Text;
            }
        }

        public static void setText(Control control, String text) {
            if (control.InvokeRequired) {
                control.Invoke(mSetTextDelegate, new object[] { control, text });
            } else {
                control.Text = text;
            }
        }
    }
}
