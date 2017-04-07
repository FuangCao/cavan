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
                if (mAddressCount > 0) {
                    return CavanMacAddress.add(mAddressStart, mAddressCount - 1);
                }

                return new CavanMacAddress(mAddressStart);
            }
        }

        public CavanMacAddress AddressNext {
            get {
                return CavanMacAddress.add(mAddressStart, mAddressCount);
            }
        }
    }
}
