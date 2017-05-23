using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace JsonGenerator {

    class ControlListView : Panel {

        public void addControl(Control control) {
            Controls.Add(control);
            updateControlBounds();
        }

        public void removeControl(Control control) {
            Controls.Remove(control);
            updateControlBounds();
        }

        public void updateControlBounds() {
            ControlCollection controls = Controls;
            int count = controls.Count;
            if (count < 1) {
                return;
            }

            int width = Width / count;
            int x = 0;

            foreach (Control control in controls) {
                Rectangle bounds = control.Bounds;

                bounds.X = x;
                bounds.Y = 0;
                bounds.Width = width - 1;
                bounds.Height = Height;

                control.Bounds = bounds;

                x += width;
            }
        }
    }
}
