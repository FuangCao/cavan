package com.cavan.cavanjni;


public class CavanJni extends CavanNative {
	public static final String TAG = "Cavan";

	public static native boolean kill(String name);
	public static native boolean setEnv(String key, String value);

	public static String getEnv(String name) {
		return System.getenv(name);
	}

	public static boolean setPathEnv(String value) {
		return setEnv("PATH", value);
	}

	public static boolean appendPathEnv(String value) {
		String path = getEnv("PATH");
		if (path == null) {
			path = value;
		} else {
			path += ":" + value;
		}

		return setPathEnv(path);
	}

	public static int doCommand(ICavanCommand command, String... args) {
		return command.main(args);
	}

	public static int doCommand(String name, String... args) {
		ICavanCommand command = sHashMap.get(name);
		if (command != null) {
			return doCommand(command, args);
		}

		return -1;
	}
}
