package com.cavan.cavanmain;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

import com.cavan.cavanutils.CavanUtils;

import android.hardware.SensorManager;
import android.os.Build;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;

public class MainActivity extends PreferenceActivity {

	private static final String KEY_IP_ADDRESS = "ip_address";

	private Preference mPreferenceIpAddress;

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.cavan_service);

		mPreferenceIpAddress = findPreference(KEY_IP_ADDRESS);
		updateIpAddressStatus();

		if (releaseCavanMain()) {
			CavanUtils.logD("releaseCavanMain OK");
		} else {
			CavanUtils.logE("releaseCavanMain Failed");
		}
	}

	@SuppressWarnings("deprecation")
	private boolean releaseCavanMain() {
		File dirBin = getDir("bin", 0777);
		if (dirBin == null) {
			CavanUtils.logE("Failed to getDir bin");
			return false;
		}

		CavanUtils.appendPathEnv(dirBin.getPath());

		File fileCavanMain = new File(dirBin, "cavan-main");
		if (fileCavanMain.canExecute()) {
			return true;
		}

		for (String abi : new String[] { Build.CPU_ABI, Build.CPU_ABI2 }) {
			InputStream inStream = null;
			OutputStream outStream = null;

			try {
				String filename = "cavan-main" + "." + abi;

				inStream = getAssets().open(filename);
				if (inStream == null) {
					continue;
				}

				CavanUtils.logD(filename + " => " + fileCavanMain.getPath());

				outStream = new FileOutputStream(fileCavanMain);

				byte[] buff = new byte[1024];

				while (true) {
					int length = inStream.read(buff);
					if (length < 1) {
						break;
					}

					outStream.write(buff, 0, length);
				}

				fileCavanMain.setExecutable(true);

				return true;
			} catch (IOException e) {
				e.printStackTrace();
			} finally {
				if (inStream != null) {
					try {
						inStream.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}

				if (outStream != null) {
					try {
						outStream.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			}
		}

		return false;
	}

	private void updateIpAddressStatus() {
		Enumeration<NetworkInterface> enNetIf;
		try {
			enNetIf = NetworkInterface.getNetworkInterfaces();
			if (enNetIf == null) {
				return;
			}
		} catch (SocketException e) {
			e.printStackTrace();
			return;
		}

		StringBuilder builder = new StringBuilder();

		while (enNetIf.hasMoreElements()) {
			Enumeration<InetAddress> enAddr = enNetIf.nextElement().getInetAddresses();
			while (enAddr.hasMoreElements()) {
				InetAddress addr = enAddr.nextElement();
				if (addr.isLoopbackAddress()) {
					continue;
				}

				if (addr.isLinkLocalAddress()) {
					continue;
				}

				if (builder.length() > 0) {
					builder.append("\n");
				}

				builder.append(addr.getHostAddress());
			}
		}

		if (builder.length() > 0) {
			mPreferenceIpAddress.setSummary(builder.toString());
		} else {
			mPreferenceIpAddress.setSummary(R.string.text_unknown);
		}
	}

	@SuppressWarnings("deprecation")
	@Override
	public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
		if (preference == mPreferenceIpAddress) {
			updateIpAddressStatus();
		}

		return super.onPreferenceTreeClick(preferenceScreen, preference);
	}
}
