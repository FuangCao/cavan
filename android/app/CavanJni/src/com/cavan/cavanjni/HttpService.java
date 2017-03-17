package com.cavan.cavanjni;

import android.content.Context;
import android.os.storage.StorageManager;

import com.cavan.android.CavanAndroid;
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
	protected void onServiceStateChanged(int state) {
		if (state == STATE_PREPARE) {
			CavanFile apk = new CavanFile(getCacheDir(), "apk");
			if (CavanJni.symlinkApks(getPackageManager(), apk)) {
				CavanJni.setEnv("APP_PATH", apk.getPath());
			}

			StorageManager manager = (StorageManager) getSystemService(Context.STORAGE_SERVICE);
			if (manager != null) {
				String[] volumes = CavanAndroid.getVolumePaths(manager);
				if (volumes != null) {
					for (int i = 0; i < volumes.length; i++) {
						CavanJni.setEnv("SDCARD" + i + "_PATH", volumes[i]);
					}
				}
			}
		}

		super.onServiceStateChanged(state);
	}
}