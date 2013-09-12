package com.cavan.touchscreen;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import android.util.Log;

public abstract class TouchscreenDevice {
	private static final String TAG = "Cavan";

	public abstract String getDevPath();
	public abstract String getDevName();
	public abstract String getFwName();
	public abstract DeviceID readDevID();

	public DeviceID readDevID(String pathname) {
		FileInputStream stream;
		try {
			stream = new FileInputStream(pathname);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			return null;
		}

		byte[] buff = new byte[4];
		int iRdLen;
		try {
			iRdLen = stream.read(buff);
		} catch (IOException e) {
			e.printStackTrace();
			iRdLen = -1;
		} finally {
			try {
				stream.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		if (iRdLen < buff.length) {
			return null;
		}

		int value = Integer.parseInt(new String(buff), 16);

		return new DeviceID(((value >> 8)) & 0xFF, value & 0xFF);
	}

	public boolean isAttach()
	{
		File file = new File(getDevPath());

		return file != null && file.exists();
	}

	public boolean upgradeFirmware(String pathname) {
		Log.d(TAG, "pathname = " + pathname);
		FileInputStream inputStream;
		try {
			inputStream = new FileInputStream(pathname);
		} catch (FileNotFoundException e2) {
			e2.printStackTrace();
			return false;
		}

		FileOutputStream outputStream;
		try {
			outputStream = new FileOutputStream(getDevPath());
		} catch (FileNotFoundException e2) {
			e2.printStackTrace();

			try {
				inputStream.close();
			} catch (IOException e) {
				e.printStackTrace();
			}

			return false;
		}

		boolean result;

		try {
			while (true) {
				byte[] buff = new byte[1024];
				int iReadLen = inputStream.read(buff);
				if (iReadLen < 0) {
					break;
				}

				outputStream.write(buff, 0, iReadLen);
			}

			result = true;
		} catch (Exception e) {
			e.printStackTrace();
			result = false;
		} finally {
			try {
				inputStream.close();
			} catch (IOException e) {
				e.printStackTrace();
			}

			try {
				outputStream.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		return result;
	}
}