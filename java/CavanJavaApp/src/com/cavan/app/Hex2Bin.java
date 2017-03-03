package com.cavan.app;

import com.cavan.java.CavanFile;
import com.cavan.java.CavanHexFile;
import com.cavan.java.CavanJava;

public class Hex2Bin {

	public static void main(String[] args) throws Exception {
		String message = null;

		if (args.length > 0) {
			CavanHexFile inFile = new CavanHexFile(args[0]);
			CavanFile outFile;

			if (args.length > 1) {
				outFile = new CavanFile(args[1]);
			} else {
				outFile = new CavanFile(inFile.getPathName("bin"));
			}

			CavanJava.dLog("convert: " + inFile.getPath() + " => " + outFile.getPath());

			byte[] bytes = inFile.parse((byte) 0xFF);
			if (bytes == null) {
				message = "Failed to parse: " + inFile.getPath();
			} else if (outFile.write(bytes) < 0) {
				message = "Failed to write: " + outFile.getPath();
			} else {
				CavanJava.dLog("convert successfull");
			}
		} else {
			message = "hex2bin <INPUT_FILE> [OUTPUT_FILE]";
		}

		if (message != null) {
			throw new Exception(message);
		}
	}
}
