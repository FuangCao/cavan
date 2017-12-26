package com.cavan.cavanjni;

import java.io.File;
import java.util.HashMap;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanAndroidFile;
import com.cavan.service.CavanServiceState;


public class TcpDdService extends CavanNativeService {

	public static final String NAME = "TCP_DD";
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

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	protected void doMainLoop(int port) {
		CavanJni.doTcpDdServer("-p", Integer.toString(port), "-s", "0");
	}

	@Override
	public int getDefaultPort() {
		return 9898;
	}

	@Override
	public boolean doStopService() {
		return CavanJni.kill("tcp_dd_server");
	}

	@Override
	protected void onServiceStateChanged(int state) {
		if (state == CavanServiceState.PREPARE) {
			File dir = getDir("bin", MODE_PRIVATE);
			if (dir != null) {
				int id = getResourceByAbi();
				if (id > 0) {
					CavanAndroidFile file = new CavanAndroidFile(dir, "cavan-main");
					if (file.copyFrom(getResources(), id)) {
						file.setExecutable(true);
						CavanJni.appendPathEnv(dir.getPath());
					}
				}
			}
		}

		super.onServiceStateChanged(state);
	}
}