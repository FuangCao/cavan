using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace NetworkInputMethod
{
    public class CavanTreeView : TreeView
    {
        protected override CreateParams CreateParams
        {
            get
            {
                var value = base.CreateParams;
                value.Style |= 0x80;
                return value;
            }
        }
    }
}
