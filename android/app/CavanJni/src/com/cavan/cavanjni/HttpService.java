package com.cavan.cavanjni;

import android.content.Context;
import android.os.storage.StorageManager;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanFile;
import com.cavan.service.CavanServiceState;

public class HttpService extends CavanNativeService {

	public static final String NAME = "HTTP";

	public HttpService() {
		super(new CavanCommandHttpService());
	}

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	protected String[] getCommandArgs(int port) {
		return new String[] { "-p", Integer.toString(port) };
	}

	@Override
	public int getDefaultPort() {
		return 8021;
	}

	public static CavanFile getSharedDir(Context context) {
		return new CavanFile(context.getCacheDir(), "shared");
	}

	public static CavanFile getAppDir(Context context) {
		return new CavanFile(context.getCacheDir(), "apk");
	}

	@Override
	protected void onServiceStateChanged(int state) {
		if (state == CavanServiceState.PREPARE) {
			CavanFile apk = getAppDir(this);
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

			CavanFile shared = getSharedDir(this);
			if (shared.mkdirSafe()) {
				shared.clear();
				CavanJni.setEnv("SHARED_PATH", shared.getPath());
			}
		}

		super.onServiceStateChanged(state);
	}
}
