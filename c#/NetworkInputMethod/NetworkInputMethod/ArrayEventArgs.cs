using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace NetworkInputMethod
{
    public class ArrayEventArgs : EventArgs
    {
        private object[] mArgs;

        public ArrayEventArgs(params object[] args)
        {
            mArgs = args;
        }

        public object[] Args
        {
            get
            {
                return mArgs;
            }

            set
            {
                mArgs = value;
            }
        }

        public object getArg(int index)
        {
            return mArgs[index];
        }
    }
}
