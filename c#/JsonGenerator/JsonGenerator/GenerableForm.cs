using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace JsonGenerator {
    public class GenerableForm : Form {
        private GenerableButton mButton;

        public GenerableButton GenerableButton {
            get {
                return mButton;
            }

            set {
                mButton = value;
            }
        }

        public virtual StringBuilder generate(StringBuilder builder, String prefix, int index) {
            return builder;
        }

        public virtual DialogResult ShowDialog(bool deleteEn) {
            return ShowDialog();
        }
    }
}
