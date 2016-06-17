package com.cavan;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

public class CavanCommand {
	private ProcessBuilder mBuilder;
	private boolean mStderrDefault = true;
	private boolean mStdoutDefault = true;

	public CavanCommand(String... command) {
		super();
		mBuilder = new ProcessBuilder(command);
	}

	public void redirectErrorStream(boolean enable) {
		mBuilder.redirectErrorStream(enable);
		mStderrDefault = false;
	}

	public void closeStdout() {
		mBuilder.redirectOutput(new File("/dev/null"));
		mStdoutDefault = false;
	}

	public void closeStderr() {
		mBuilder.redirectError(new File("/dev/null"));
		mStderrDefault = false;
	}

	public boolean doExec() {
		try {
			if (mStderrDefault) {
				mBuilder.redirectError(new File("/dev/stderr"));
			}

			if (mStdoutDefault) {
				mBuilder.redirectOutput(new File("/dev/stdout"));
			}

			Process process = mBuilder.start();
			return process.waitFor() == 0;
		} catch (Exception e) {
			e.printStackTrace();
		}

		return false;
	}

	public List<String> doPipe() {
		try {
			if (mStderrDefault) {
				mBuilder.redirectError(new File("/dev/stderr"));
			}

			Process process = mBuilder.start();

			BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
			List<String> lines = new ArrayList<String>();

			while (true) {
				String line = reader.readLine();
				if (line == null) {
					break;
				}

				lines.add(line);
			}

			reader.close();

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