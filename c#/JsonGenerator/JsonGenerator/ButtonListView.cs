using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace JsonGenerator {

    public class ButtonListView : Panel {
        public void addButton(GenerableButton button) {
            Controls.Add(button);
            updateButtonBounds();
        }

        public void removeButton(GenerableButton button) {
            Controls.Remove(button);
            updateButtonBounds();
        }

        public int addOpenFileButtons(OpenFileDialog dialog, ContextMenuStrip menu, ToolTip toolTip) {
            dialog.Multiselect = true;

            if (dialog.ShowDialog() != DialogResult.OK) {
                return 0;
            }

            int count = 0;

            if (dialog.Multiselect) {
                foreach (String filename in dialog.FileNames) {
                    OpenFileButton button = new OpenFileButton(this, dialog, filename, menu, toolTip);
                    Controls.Add(button);
                    count++;
                }
            } else {
                OpenFileButton button = new OpenFileButton(this, dialog, dialog.FileName, menu, toolTip);
                Controls.Add(button);
                count++;
            }

            updateButtonBounds();

            return count;
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

        public static StringBuilder generate(StringBuilder builder, string prefix, string name, params ButtonListView[] lists) {
            string subPrefix = prefix + "    ";
            int fileIndex = 0;
            int formIndex = 0;

            builder.Append(prefix).Append('"').Append(name).Append("\":[");

            foreach (ButtonListView list in lists) {
                foreach (Control control in list.Controls) {
                    if (control is OpenFileButton) {
                        OpenFileButton button = (OpenFileButton)control;
                        if (fileIndex > 0) {
                            builder.Append(',');
                        }

                        button.generate(builder, subPrefix, fileIndex++);
                    } else if (control is OpenDialogButton) {
                        OpenDialogButton button = (OpenDialogButton)control;
                        if (formIndex > 0) {
                            builder.Append(',');
                        }

                        builder.AppendLine();

                        button.generate(builder, subPrefix, formIndex++);
                    }
                }
            }

            if (formIndex > 0) {
                builder.AppendLine().Append(prefix);
            }

            builder.Append(']');

            return builder;
        }

        public StringBuilder generate(StringBuilder builder, string prefix, string name) {
            return generate(builder, prefix, name, this);
        }
    }
}
