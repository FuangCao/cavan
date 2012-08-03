package com.cavan.calculator2;

public class FormulaCalculator
{
	public FormulaCalculator()
	{
	}
	
	public static long ByteToLong(byte b)
	{
		if (b >= '0' && b <= '9')
		{
			return b - '0';
		}
		else if (b >= 'a' && b <= 'z')
		{
			return b - 'a' + 10;
		}
		else if (b >= 'A' && b <= 'Z')
		{
			return b - 'A' + 10;
		}
		else
		{
			return -1;
		}
	}

	public static byte LongToByte(long value)
	{
		if (value >= 0 && value <= 9)
		{
			return (byte) (value + '0');
		}

		return (byte) (value + 'A' - 10);
	}

	public static byte[] ByteArrayExchange(byte[] bytes, int index, int size)
	{
		for (int last = index + size - 1; index < last; index++, last--)
		{
			byte temp = bytes[index];
			bytes[index] = bytes[last];
			bytes[last] = temp;
		}

		return bytes;
	}

	public static String LongToText(long value, int base, int size)
	{
		if (value == 0 && size == 0)
		{
			return "0";
		}

		int i;
		byte[] bytes = new byte[128];

		for (i = 0; value != 0; value /= base, i++)
		{
			bytes[i] = LongToByte(value % base);
		}

		for (size -= i; size > 0; size--, i++)
		{
			bytes[i] = '0';
		}

		return new String(ByteArrayExchange(bytes, 0, i), 0, i);
	}

	public static long TextToLong(String text, int base)
	{
		long value = 0;
		byte[] bytes = text.getBytes();

		for (int i = 0; i < bytes.length; i++)
		{
			long temp = ByteToLong(bytes[i]);
			if (temp < 0 || temp >= base)
			{
				break;
			}

			value = value * base + temp;
		}

		return value;
	}
}
