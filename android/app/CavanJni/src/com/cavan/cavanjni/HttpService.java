package com.cavan.cavanjni;

import com.cavan.java.CavanFile;

public class HttpService extends CavanService {

	public static final String NAME = "HTTP";

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	protected void doMainLoop(int port) {
		CavanJni.doHttpService("-p", Integer.toString(port));
	}

	@Override
	public int getDefaultPort() {
		return 8021;
	}

	@Override
	public boolean doStopService() {
		return CavanJni.kill("http_service");
	}

	@Override
	protected void onServiceStart() {
		super.onServiceStart();

		CavanFile apk = new CavanFile(getCacheDir(), "apk");
		if (CavanJni.symlinkApks(getPackageManager(), apk)) {
			CavanJni.setEnv("APP_PATH", apk.getPath());
		}
	}
}