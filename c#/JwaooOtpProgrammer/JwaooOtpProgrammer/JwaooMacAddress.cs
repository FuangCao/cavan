using System;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace JwaooOtpProgrammer {
    public class JwaooMacAddress : CavanMacAddress {

        private String mFilePath;
        private String mFwPrefix;
        private bool mLoadSucceed;
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

            set {
                mAddressCount = value;
            }
        }

        public String AddressCountText {
            get {
                return Convert.ToString(mAddressCount);
            }

            set {
                try {
                    mAddressCount = Convert.ToUInt32(value);
                } catch (Exception) {
                    mAddressCount = 0;
                }
            }
        }

        public CavanMacAddress AddressStart {
            get {
                return mAddressStart;
            }

            set {
                mAddressStart = value;
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

        public bool LoadSucceed {
            get {
                return mLoadSucceed;
            }

            set {
                mLoadSucceed = value;
            }
        }

        public bool isAddressValid(CavanMacAddress address) {
            return address.startsWith(mAddressStart);
        }

        public JwaooMacAddress fromStringWithCount(String text) {
            String[] texts = Regex.Split(text, "\\s+");
            if (texts.Length > 1) {
                AddressCountText = texts[1];
                text = texts[0];
            } else {
                mAddressCount = 0;
            }

            CavanMacAddress address = fromString(text);
            if (isAddressValid(address)) {
                return this;
            }

            return null;
        }

        public bool readFromFile() {
            mLoadSucceed = false;

            StreamReader reader = null;

            try {
                reader = File.OpenText(mFilePath);

                JwaooMacAddress address = fromStringWithCount(reader.ReadLine());
                if (address != null) {
                    mLoadSucceed = true;
                } else {
                    MessageBox.Show("MAC地址配置文件格式错误，应该类似：" + mAddressStart + " 100");
                }
            } catch (FileNotFoundException) {
                copyFrom(mAddressStart);
                if (writeToFile()) {
                    mLoadSucceed = true;
                }
            } catch (Exception e) {
                MessageBox.Show("读MAC地址出错：\n" + e);
            } finally {
                if (reader != null) {
                    reader.Close();
                }
            }

            return mLoadSucceed;
        }

        public bool writeToFile() {
            StreamWriter writer = null;

            try {
                writer = File.CreateText(mFilePath);
                writer.Write(ToString() + " " + mAddressCount);
            } catch (Exception e) {
                MessageBox.Show("保存MAC地址出错：\n" + e);
                return false;
            } finally {
                if (writer != null) {
                    writer.Close();
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
            mAddressCount++;

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
                writeToFile();
                return true;
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
