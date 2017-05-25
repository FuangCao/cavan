using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace JsonGenerator {
    public class GenerableButton : Button {
        public virtual StringBuilder generate(StringBuilder builder, String prefix, int index) {
            return builder;
        }
    }
}
