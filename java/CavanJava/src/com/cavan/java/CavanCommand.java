package com.cavan.java;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

public class CavanCommand {
	private ProcessBuilder mBuilder;
	private boolean mStderrDefault = true;

	public CavanCommand(String... command) {
		super();
		mBuilder = new ProcessBuilder(command);
	}

	public void redirectErrorStream(boolean enable) {
		mBuilder.redirectErrorStream(enable);
		mStderrDefault = false;
	}

	public boolean redirectOut(File file) {
		return CavanJava.invokeMethod(mBuilder, "redirectOutput", file) != null;
	}

	public boolean redirectErr(File file) {
		return CavanJava.invokeMethod(mBuilder, "redirectError", file) != null;
	}

	public boolean redirectOut(String pathname) {
		return redirectOut(new File(pathname));
	}

	public boolean redirectErr(String pathname) {
		return redirectErr(new File(pathname));
	}

	public boolean closeOut() {
		return redirectOut("/dev/null");
	}

	public boolean closeErr() {
		if (!redirectErr("/dev/null")) {
			return false;
		}

		mStderrDefault = false;

		return true;
	}

	public boolean doExec() {
		try {
			if (mStderrDefault && !redirectErr("/dev/stderr")) {
				mBuilder.redirectErrorStream(true);
			}

			Process process = mBuilder.start();
			InputStream stream = process.getInputStream();
			if (stream != null) {
				byte[] bytes = new byte[64];

				while (true) {
					int length = stream.read(bytes);
					if (length <= 0) {
						if (length < 0) {
							break;
						}

						continue;
					}

					System.out.write(bytes, 0, length);
				}
			}

			return process.waitFor() == 0;
		} catch (Exception e) {
			e.printStackTrace();
		}

		return false;
	}

	public List<String> doPipe() {
		try {
			if (mStderrDefault && !redirectErr("/dev/stderr")) {
				mBuilder.redirectErrorStream(true);
			}

			Process process = mBuilder.start();
			List<String> lines = new ArrayList<String>();
			InputStream stream = process.getInputStream();
			if (stream != null) {
				BufferedReader reader = new BufferedReader(new InputStreamReader(stream));

				while (true) {
					String line = reader.readLine();
					if (line == null) {
						break;
					}

					lines.add(line);
				}

				reader.close();
			}

			if (process.waitFor() != 0) {
				return null;
			}

			return lines;
		} catch (Exception e) {
			e.printStackTrace();
		}

		return null;
	}
}
