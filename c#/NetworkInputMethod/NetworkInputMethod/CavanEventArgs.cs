using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace NetworkInputMethod
{
    public class CavanEventArgs<E> : EventArgs
    {
        private E mArgs;

        public CavanEventArgs(E args)
        {
            mArgs = args;
        }

        public E Args
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
    }
}
