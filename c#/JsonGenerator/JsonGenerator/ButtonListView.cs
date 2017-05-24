using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

namespace JsonGenerator {

    public class ButtonListView : Panel {
        public void addButton(Button button) {
            Controls.Add(button);
            updateButtonBounds();
        }

        public void removeButton(Button button) {
            Controls.Remove(button);
            updateButtonBounds();
        }

        public void updateButtonBounds() {
            ControlCollection controls = Controls;
            int count = controls.Count;
            if (count < 1) {
                return;
            }

            int width = 0;
            Graphics graphics = CreateGraphics();

            foreach (Control control in controls) {
                SizeF size = graphics.MeasureString(control.Text, control.Font);
                Padding margin = control.Margin;

                control.Width = (int)size.Width + margin.Left + margin.Right + 5;
                width += control.Width;
            }

            graphics.Dispose();

            if (width > Width) {
                int avgWidth = Width / count;
                List<Control> list = new List<Control>();

                width = 0;

                foreach (Control control in controls) {
                    if (control.Width > avgWidth) {
                        list.Add(control);
                    } else {
                        width += control.Width;
                    }
                }

                avgWidth = (Width - width) / list.Count;

                foreach (Control control in list) {
                    control.Width = avgWidth;
                }
            }

            int x = 0;

            foreach (Control control in controls) {
                Rectangle bounds = control.Bounds;

                bounds.X = x;
                bounds.Y = 0;
                bounds.Height = Height;

                control.Bounds = bounds;

                x += bounds.Width;
            }
        }
    }
}
