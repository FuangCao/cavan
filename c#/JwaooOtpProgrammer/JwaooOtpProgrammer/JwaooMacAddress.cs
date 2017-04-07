using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace JwaooOtpProgrammer {
    class JwaooMacAddress : CavanMacAddress {

        private UInt32 mCount;
        private String mPathname;
        private CavanMacAddress mAddressStart;

        public JwaooMacAddress(CavanMacAddress address, String pathname) {
            mAddressStart = address;
            mPathname = pathname;
        }

        public UInt32 Count {
            get {
                return mCount;
            }
        }

        public bool setCount(String text) {
            try {
                mCount = Convert.ToUInt32(text);
            } catch {
                mCount = 0;
            }

            return false;
        }

        public new JwaooMacAddress fromString(String text) {
            String[] texts = Regex.Split(text, "\\s+");
            if (texts.Length > 1) {
                setCount(texts[1]);
                text = texts[0];
            } else {
                mCount = 0;
            }

            CavanMacAddress address = base.fromString(text);
            if (address.startsWith(mAddressStart)) {
                return this;
            }

            return null;
        }

        public JwaooMacAddress readFromFile() {
            FileStream stream = null;

            try {
                stream = File.OpenRead(mPathname);

                byte[] buff = new byte[32];
                int length = stream.Read(buff, 0, buff.Length);
                String text = Encoding.ASCII.GetString(buff, 0, length);

                JwaooMacAddress address = fromString(text);
                if (address != null) {
                    return address;
                }

                MessageBox.Show("MAC地址格式错误：" + mPathname);
            } catch (FileNotFoundException) {
                copyFrom(mAddressStart);
                if (writeToFile()) {
                    return this;
                }
            } catch (Exception e) {
                MessageBox.Show("读MAC地址出错：\n" + e);
            } finally {
                if (stream != null) {
                    stream.Close();
                }
            }

            return null;
        }

        public bool writeToFile() {
            String text = ToString() + " " + mCount;
            byte[] bytes = Encoding.ASCII.GetBytes(text);

            FileStream stream = null;

            try {
                stream = File.OpenWrite(mPathname);
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
            if (mCount > 0) {
                mCount--;
            } else {
                return false;
            }

            if (base.increase() == 0) {
                return true;
            }

            base.decrease();
            mCount--;

            return false;
        }

        public new bool decrease() {
            if (base.decrease() != 0) {
                base.increase();
                return false;
            }

            mCount++;

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
    }
}
