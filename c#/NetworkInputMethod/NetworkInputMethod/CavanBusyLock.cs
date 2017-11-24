using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace NetworkInputMethod
{
    public class CavanBusyLock
    {
        private int mOverTime;
        private object mLockedObj;
        private DateTime mLockedTime = DateTime.Now;

        public CavanBusyLock(int overtime)
        {
            mOverTime = overtime;
        }

        public CavanBusyLock()
        {
            mOverTime = 200;
        }

        public bool acquire(object obj)
        {
            DateTime now = DateTime.Now;

            lock (this)
            {
                if (obj != mLockedObj && mLockedObj != null)
                {
                    TimeSpan span = now - mLockedTime;

                    if (span.TotalMilliseconds < mOverTime)
                    {
                        return false;
                    }
                }

                mLockedObj = obj;
                mLockedTime = now;
            }

            return true;
        }

        public bool release(object obj)
        {
            lock (this)
            {
                if (mLockedObj == obj)
                {
                    mLockedObj = null;
                    return true;
                }
            }

            return false;
        }
    }
}
