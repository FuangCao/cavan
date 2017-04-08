using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace JwaooOtpProgrammer {
    class JwaooMacAddress : CavanMacAddress {

        private String mFilePath;
        private String mFwPrefix;
        private UInt32 mAddressCount;
        private CavanMacAddress mAddressStart;

        public JwaooMacAddress() { }

        public JwaooMacAddress(String fileName, String fwPrefix, CavanMacAddress startAddress) {
            mFilePath = Path.Combine(Application.StartupPath, fileName);
            mFwPrefix = fwPrefix;
            mAddressStart = startAddress;
        }

        public String FilePath {
            get {
                return mFilePath;
            }

            set {
                mFilePath = value;
            }
        }

        public String FwPrefix {
            get {
                return mFwPrefix;
            }

            set {
                mFwPrefix = value;
            }
        }

        public UInt32 AddressCount {
            get {
                return mAddressCount;
            }
        }

        public CavanMacAddress AddressEnd {
            get {
                return getAddressEnd(mAddressCount);
            }
        }

        public CavanMacAddress AddressNext {
            get {
                return getAddressNext(mAddressCount);
            }
        }

        public bool setAddressCount(String text) {
            try {
                mAddressCount = Convert.ToUInt32(text);
            } catch {
                mAddressCount = 0;
            }

            return false;
        }

        public new JwaooMacAddress fromString(String text) {
            String[] texts = Regex.Split(text, "\\s+");
            if (texts.Length > 1) {
                setAddressCount(texts[1]);
                text = texts[0];
            } else {
                mAddressCount = 0;
            }

            CavanMacAddress address = base.fromString(text);
            if (address.startsWith(mAddressStart)) {
                return this;
            }

            return null;
        }

        public bool readFromFile() {
            FileStream stream = null;

            try {
                stream = File.OpenRead(mFilePath);

                byte[] buff = new byte[32];
                int length = stream.Read(buff, 0, buff.Length);
                String text = Encoding.ASCII.GetString(buff, 0, length);

                JwaooMacAddress address = fromString(text);
                if (address != null) {
                    return true;
                }

                MessageBox.Show("MAC地址配置文件格式错误，应该类似：" + mAddressStart + " 100");
            } catch (FileNotFoundException) {
                copyFrom(mAddressStart);
                if (writeToFile()) {
                    return true;
                }
            } catch (Exception e) {
                MessageBox.Show("读MAC地址出错：\n" + e);
            } finally {
                if (stream != null) {
                    stream.Close();
                }
            }

            return false;
        }

        public bool writeToFile() {
            String text = ToString() + " " + mAddressCount;
            byte[] bytes = Encoding.ASCII.GetBytes(text);

            FileStream stream = null;

            try {
                stream = File.OpenWrite(mFilePath);
                stream.SetLength(0);
                stream.Write(bytes, 0, bytes.Length);
            } catch (Exception e) {
                MessageBox.Show("保存MAC地址出错：\n" + e);
                return false;
            } finally {
                if (stream != null) {
                    stream.Close();
                }
            }

            return true;
        }

        public new bool increase() {
            if (mAddressCount > 0) {
                mAddressCount--;
            } else {
                return false;
            }

            if (base.increase() == 0) {
                return true;
            }

            base.decrease();
            mAddressCount--;

            return false;
        }

        public new bool decrease() {
            if (base.decrease() != 0) {
                base.increase();
                return false;
            }

            mAddressCount++;

            return true;
        }

        public bool increaseAndSave() {
            if (increase()) {
                if (writeToFile()) {
                    return true;
                }

                decrease();
            }

            return false;
        }

        public JwaooMacAddress fromOtpMemory(byte[] bytes, int index) {
            return (JwaooMacAddress)fromBytes(bytes, index, index + 6);
        }

        public JwaooMacAddress fromOtpHeader(byte[] bytes) {
            return fromOtpMemory(bytes, 0xD4);
        }

        public JwaooMacAddress fromOtpFirmware(byte[] bytes) {
            return fromOtpMemory(bytes, 0x7FD4);
        }

        public String toStringOtp() {
            return toStringHexReverse();
        }
    }
}
