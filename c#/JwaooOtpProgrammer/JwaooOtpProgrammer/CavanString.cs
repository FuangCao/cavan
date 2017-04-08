using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace JwaooOtpProgrammer {
    class CavanString {

        public static readonly char[] NUMBER_UPPERCASE_CHARS = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
        public static readonly char[] NUMBER_LOWERCASE_CHARS = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

        public static char ValueToCharUppercase(int value) {
            return NUMBER_UPPERCASE_CHARS[value];
        }

        public static char ValueToCharLowercase(int value) {
            return NUMBER_LOWERCASE_CHARS[value];
        }

        public static byte CharToValue(char c) {
            if (c >= '0' && c <= '9') {
                return (byte) (c - '0');
            }

            if (c >= 'a' && c <= 'z') {
                return (byte) (c - 'a' + 10);
            }

            if (c >= 'A' && c <= 'Z') {
                return (byte) (c - 'A' + 10);
            }

            return 0;
        }

        public static void fromByte(char[] chars, int index, byte value) {
            chars[index] = ValueToCharUppercase((value >> 4) & 0x0F);
            chars[index + 1] = ValueToCharUppercase(value & 0x0F);
        }

        public static StringBuilder fromByte(StringBuilder build, byte value) {
            return build.Append(ValueToCharUppercase((value >> 4) & 0x0F)).Append(ValueToCharUppercase(value & 0x0F));
        }

        public static String fromByte(byte value) {
            char[] chars = new char[2];
            fromByte(chars, 0, value);

            return new String(chars);
        }
    }
}
