using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace JwaooOtpProgrammer {

    public class CavanMacAddress : CavanLargeValue {

        public static readonly char SEPRATOR = ':';

        private char mSeparator;

        public CavanMacAddress(byte[] bytes, char separator) : base(bytes) {
            mSeparator = separator;
        }

        public CavanMacAddress(byte[] bytes) : this(bytes, SEPRATOR) { }

        public CavanMacAddress(int length, char separator) : base(length) {
            mSeparator = separator;
        }

        public CavanMacAddress(int length) : this(length, SEPRATOR) { }

        public CavanMacAddress() : this(6) { }

        public CavanMacAddress(CavanMacAddress address) : base(address) {
            mSeparator = address.getSeparator();
        }

        public void setSeparator(char separator) {
            mSeparator = separator;
        }

        public char getSeparator() {
            return mSeparator;
        }

        public CavanMacAddress copyFrom(CavanMacAddress address) {
            base.copyFrom(address);
            mSeparator = address.getSeparator();
            return this;
        }

        public CavanMacAddress copyTo(CavanMacAddress address) {
            base.copyTo(address);
            address.setSeparator(mSeparator);
            return address;
        }

        public new CavanMacAddress fromValues(params int[] args) {
            return (CavanMacAddress)base.fromValues(args);
        }

        public new CavanMacAddress fromStrings(params String[] args) {
            return (CavanMacAddress)base.fromStrings(args);
        }

        public new CavanMacAddress fromString(String text) {
            return (CavanMacAddress)base.fromString(text, mSeparator);
        }

        public static CavanMacAddress add(CavanMacAddress left, long value) {
            left = left.cloneMacAddress();
            left.add(value);
            return left;
        }

        public static CavanMacAddress sub(CavanMacAddress address, long value) {
            address = address.cloneMacAddress();
            address.sub(value);
            return address;
        }

        public static CavanMacAddress sub(CavanMacAddress left, CavanMacAddress right) {
            CavanMacAddress address = left.cloneMacAddress();
            address.sub(right);
            return address;
        }

        public CavanMacAddress cloneMacAddress() {
            return new CavanMacAddress(this);
        }

        public CavanMacAddress getAddressEnd(long count) {
            CavanMacAddress address = cloneMacAddress();

            if (count > 0) {
                address.add(count - 1);
            } else if (address.decrease() != 0) {
                address.increase();
            }

            return address;
        }

        public CavanMacAddress getAddressNext(long count) {
            CavanMacAddress address = cloneMacAddress();
            address.add(count);
            return address;
        }

        public override String ToString() {
            return toString(mSeparator);
        }
    }
}
