package com.cavan.java;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.net.URI;

public class CavanHexFile extends CavanFile {

	private static final long serialVersionUID = 4346572307027004468L;

	private long mBaseAddress;

	public CavanHexFile(File dir, String name) {
		super(dir, name);
	}

	public CavanHexFile(String dirPath, String name) {
		super(dirPath, name);
	}

	public CavanHexFile(String path) {
		super(path);
	}

	public CavanHexFile(URI uri) {
		super(uri);
	}

	public byte[] parse(BufferedReader reader, byte fill) {
		try {
			int sizeReal = 0;
			byte[] binData = new byte[(int) length()];

			while (true) {
				String line = reader.readLine();
				if (line == null) {
					break;
				}

				if (line.charAt(0) != ':') {
					return null;
				}

				byte[] bytes = CavanJava.parseHexText(line.getBytes(), 1, line.length() - 1);
				int length = bytes[0];
				int offset = (bytes[1] & 0xFF) << 8 | (bytes[2] & 0xFF);
				int type = bytes[3];

				int i = 4;
				int end = i + length;

				switch (type) {
				case 0x00:
					if (offset + length > binData.length) {
						byte[] datas = new byte[offset + length];
						CavanArray.copy(binData, datas, sizeReal);
						binData = datas;
					}

					if (offset > sizeReal) {
						for (int j = sizeReal; j < offset; j++) {
							binData[j] = fill;
						}
					}

					while (i < end) {
						binData[offset++] = bytes[i++];
					}

					if (offset > sizeReal) {
						sizeReal = offset;
					}
					break;

				case 0x01:
					return CavanArray.cloneByLength(binData, sizeReal);

				case 0x02:
					break;

				case 0x03:
					break;

				case 0x04:
					mBaseAddress = 0;
					while (i < end) {
						mBaseAddress = mBaseAddress << 4 | (bytes[i++] & 0xFF);
					}
					break;

				case 0x05:
					break;

				default:
					return null;
				}
			}
		} catch (IOException e) {
			e.printStackTrace();
		}

		return null;
	}

	public byte[] parse(BufferedReader reader) {
		return parse(reader, (byte) 0);
	}

	public byte[] parse(byte fill) {
		BufferedReader reader = openBufferedReader();
		if (reader == null) {
			return null;
		}

		byte[] data = parse(reader, fill);

		try {
			reader.close();
		} catch (IOException e) {
			e.printStackTrace();
		}

		return data;
	}

	public byte[] parse() {
		return parse((byte) 0);
	}
}
