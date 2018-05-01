package com.cavan.cavanjni;

import java.io.File;
import java.util.HashMap;

import android.os.Environment;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanAndroidFile;
import com.cavan.service.CavanServiceState;


public class TcpDdService extends CavanNativeService {

	public static final String NAME = "TCP_DD";
	public static final String CMD_NAME = "cavan-main";
	private static final HashMap<String, Integer> mResourceMap = new HashMap<String, Integer>();

	static {
		mResourceMap.put("armeabi", R.raw.cavan_main_armeabi);
		mResourceMap.put("x86", R.raw.cavan_main_x86);
	}

	private int getResourceByAbi() {
		for (String abi : CavanAndroid.getSupportedAbis()) {
			Integer id = mResourceMap.get(abi);
			if (id != null) {
				return id;
			}
		}

		return 0;
	}

	public TcpDdService() {
		super(new CavanCommandTcpDdServer());
	}

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	protected String[] getCommandArgs(int port) {
		return new String[] { "-p", Integer.toString(port), "-s", "0" };
	}

	@Override
	public int getDefaultPort() {
		return 9898;
	}

	@Override
	protected void onServiceStateChanged(int state) {
		if (state == CavanServiceState.PREPARE) {
			File dir = getDir("bin", MODE_PRIVATE);
			if (dir != null) {
				int id = getResourceByAbi();
				if (id > 0) {
					CavanAndroidFile[] files = {
						new CavanAndroidFile(dir, CMD_NAME),
						new CavanAndroidFile(Environment.getExternalStorageDirectory(), CMD_NAME),
					};

					for (CavanAndroidFile file : files) {
						CavanAndroid.dLog("copy file: " + file);

						if (file.copyFrom(getResources(), id)) {
							file.setExecutable(true);
							CavanJni.appendPathEnv(dir.getPath());
						} else {
							CavanAndroid.eLog("Failed to copy file: " + file);
						}
					}
				}
			}
		}

		super.onServiceStateChanged(state);
	}
}