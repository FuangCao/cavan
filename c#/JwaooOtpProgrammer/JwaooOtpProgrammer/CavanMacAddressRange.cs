using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace JwaooOtpProgrammer {
    class CavanMacAddressRange {

        private UInt32 mAddressCount;
        private CavanMacAddress mAddressStart;

        public CavanMacAddressRange(CavanMacAddress address, UInt32 count) {
            mAddressStart = address;
            mAddressCount = count;
        }

        public CavanMacAddressRange(CavanMacAddress address) : this(address, 0) { }

        public CavanMacAddressRange() : this(new CavanMacAddress()) { }

        public CavanMacAddress AddressStart {
            get {
                return mAddressStart;
            }

            set {
                mAddressStart = value;
            }
        }

        public UInt32 AddressCount {
            get {
                return mAddressCount;
            }

            set {
                mAddressCount = value;
            }
        }

        public CavanMacAddress AddressEnd {
            get {
                return mAddressStart.getAddressEnd(mAddressCount);
            }
        }

        public CavanMacAddress AddressNext {
            get {
                return AddressStart.getAddressNext(mAddressCount);
            }
        }
    }
}
