package com.cavan.huahardwareinfo;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.util.Log;

public class HuaHardwareInfoActivity extends PreferenceActivity {
	private static final String TAG = "Cavan";
	private static final String KEY_LCD_INFO = "lcd_info";
	private static final String KEY_TP_INFO = "tp_info";
	private static final String KEY_CAMERA_INFO = "camera_info";
	private static final String KEY_FLASH_INFO = "flash_info";
	private static final String KEY_GSENSOR_INFO = "gsensor_info";

	private File mFileLcdInfo = new File("/sys/devices/platform/sprdfb.0/dev_info");
	private File mFileCameraInfo = new File("/sys/devices/platform/sc8800g_dcam.0/dev_info");
	private File mFileFlashInfo = new File("/sys/devices/platform/sprd_nand/dev_info");

	private PreferenceCategory mPreferenceCategoryLcdInfo;
	private PreferenceCategory mPreferenceCategoryTpInfo;
	private PreferenceCategory mPreferenceCategoryCameraInfo;
	private PreferenceCategory mPreferenceCategoryFlashInfo;
	private PreferenceCategory mPreferenceCategoryGsensorInfo;
	private HuaTouchscreenDevice mTouchscreenDevice;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.hua_hardware_info);

		mPreferenceCategoryLcdInfo = (PreferenceCategory) findPreference(KEY_LCD_INFO);
		mPreferenceCategoryTpInfo = (PreferenceCategory) findPreference(KEY_TP_INFO);
		mPreferenceCategoryCameraInfo = (PreferenceCategory) findPreference(KEY_CAMERA_INFO);
		mPreferenceCategoryFlashInfo = (PreferenceCategory) findPreference(KEY_FLASH_INFO);
		mPreferenceCategoryGsensorInfo = (PreferenceCategory) findPreference(KEY_GSENSOR_INFO);

		new Thread() {
			public void run() {
				loadLcdInfo(0);
				loadTpInfo();
				loadCameraInfo();
				loadFlashInfo();
				loadGsensorInfo();
			}
		}.start();
	}

	@Override
	public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
		String key = preference.getKey();

		Log.d(TAG, "key = " + preference.getKey());

		if (KEY_LCD_INFO.equals(key)) {
			loadLcdInfo(0);
		} else if (KEY_TP_INFO.equals(key)) {
			loadTpInfo();
		} else if (KEY_CAMERA_INFO.equals(key)) {
			loadCameraInfo();
		} else if (KEY_FLASH_INFO.equals(key)) {
			loadFlashInfo();
		} else if (KEY_GSENSOR_INFO.equals(key)) {
			loadGsensorInfo();
		}

		return super.onPreferenceTreeClick(preferenceScreen, preference);
	}

	private String readFile(File file) {
		FileInputStream inputStream;
		try {
			inputStream = new FileInputStream(file);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			return  null;
		}

		String content = null;

		try {
			byte[] buff = new byte[inputStream.available()];
			content = new String(buff, 0, inputStream.read(buff));
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			try {
				inputStream.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		Log.d(TAG, content);

		return content;
	}

	private HashMap<String, String> parseFile(String content) {
		HashMap<String, String> hashMap = new HashMap<String, String>();

		for (String item : content.split("\\s*,\\s*")) {
			String[] map = item.split("\\s*=\\s*");
			if (map != null && map.length == 2) {
				hashMap.put(map[0], map[1]);
			}
		}

		return hashMap;
	}

	private HashMap<String, String> parseFile(File file) {
		String content = readFile(file);
		if (content == null) {
			return null;
		}

		return parseFile(content);
	}

	private boolean loadLcdInfo(int retry) {
		Log.w(TAG, "loadLcdInfo retry = " + retry);

		mPreferenceCategoryLcdInfo.removeAll();

		HashMap<String, String> hashMap = parseFile(mFileLcdInfo);
		if (hashMap == null) {
			return false;
		}

		List<PreferenceScreen> preferenceScreens = new ArrayList<PreferenceScreen>();

		String id = hashMap.get("id");
		if (id != null) {
			PreferenceScreen preference = mPreferenceCategoryLcdInfo.getPreferenceManager().createPreferenceScreen(this);;
			preference.setTitle(R.string.info_id);
			preference.setSummary(id);
			preferenceScreens.add(preference);

			String vendorIdContent = hashMap.get("vendor_id");
			int vendorId;
			if (vendorIdContent == null) {
				vendorId = 0;
			} else {
				vendorId = Integer.parseInt(vendorIdContent);
			}

			HuaLcdInfo lcdInfo = new HuaLcdInfo(Integer.parseInt(id, 16), vendorId);
			String ic = lcdInfo.getIc();
			if (ic != null) {
				preference = mPreferenceCategoryLcdInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(R.string.info_ic);
				preference.setSummary(ic);
				preferenceScreens.add(preference);
			} else if (retry < 20) {
				return loadLcdInfo(++retry);
			}

			HuaLcdVendorInfo vendorInfo = lcdInfo.getVendorInfo();
			if (vendorInfo != null) {
				preference = mPreferenceCategoryLcdInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(R.string.info_module_vendor);
				preference.setSummary(vendorInfo.getVendorName());
				preferenceScreens.add(preference);
			}
		}

		String width = hashMap.get("width");
		String height = hashMap.get("height");
		if (width != null && height != null) {
			PreferenceScreen preference = mPreferenceCategoryLcdInfo.getPreferenceManager().createPreferenceScreen(this);;
			preference.setTitle(R.string.info_resolution);
			preference.setSummary(width + " * " + height);
			preferenceScreens.add(preference);
		}

		String bus_width = hashMap.get("bus_width");
		if (bus_width != null) {
			PreferenceScreen preference = mPreferenceCategoryLcdInfo.getPreferenceManager().createPreferenceScreen(this);;
			preference.setTitle(R.string.info_bus_width);
			preference.setSummary(bus_width);
			preferenceScreens.add(preference);
		}

		for (PreferenceScreen preferenceScreen : preferenceScreens) {
			preferenceScreen.setKey(mPreferenceCategoryLcdInfo.getKey());
			mPreferenceCategoryLcdInfo.addPreference(preferenceScreen);
		}

		return true;
	}

	public boolean loadTpInfo() {
		mPreferenceCategoryTpInfo.removeAll();

		if (mTouchscreenDevice == null) {
			mTouchscreenDevice = HuaTouchscreenDevice.getTouchscreenDevice();

			if (mTouchscreenDevice == null) {
				return false;
			}
		}

		List<PreferenceScreen> preferenceScreens = new ArrayList<PreferenceScreen>();

		PreferenceScreen preference = mPreferenceCategoryTpInfo.getPreferenceManager().createPreferenceScreen(this);
		preference.setTitle(R.string.info_ic);
		preference.setSummary(mTouchscreenDevice.getIcName());
		preferenceScreens.add(preference);

		preference = mPreferenceCategoryTpInfo.getPreferenceManager().createPreferenceScreen(this);
		preference.setTitle(R.string.info_fw_id);
		preference.setSummary(Integer.toString(mTouchscreenDevice.getFwId(), 16));
		preferenceScreens.add(preference);

		HuaTouchscreenVendorInfo info = mTouchscreenDevice.getVendorInfo();
		if (info != null) {
			preference = mPreferenceCategoryTpInfo.getPreferenceManager().createPreferenceScreen(this);
			preference.setTitle(R.string.info_module_vendor);
			preference.setSummary(info.getVendorName());
			preferenceScreens.add(preference);
		}

		String fwName = mTouchscreenDevice.getFwName();
		if (fwName != null) {
			preference = mPreferenceCategoryTpInfo.getPreferenceManager().createPreferenceScreen(this);
			File fileFw = new File("/data", fwName);
			if (fileFw != null && fileFw.canRead()) {
				mTouchscreenDevice.setFileFw(fileFw);
				preference.setTitle(R.string.info_fw_upgrade);
				preference.setSummary(fileFw.getPath());
				preference.setOnPreferenceClickListener(new OnPreferenceClickListener() {
					@Override
					public boolean onPreferenceClick(Preference preference) {
						HuaTpUpgradeDialog dialog = new HuaTpUpgradeDialog(HuaHardwareInfoActivity.this);
						dialog.show();
						return true;
					}
				});
			} else {
				preference.setTitle(R.string.info_fw_name);
				preference.setSummary(fwName);
			}

			preferenceScreens.add(preference);
		}

		preference = mPreferenceCategoryTpInfo.getPreferenceManager().createPreferenceScreen(this);
		preference.setTitle(R.string.info_dev_path);
		preference.setSummary(mTouchscreenDevice.getFileDevice().getPath());
		preferenceScreens.add(preference);

		for (PreferenceScreen preferenceScreen : preferenceScreens) {
			preferenceScreen.setKey(mPreferenceCategoryTpInfo.getKey());
			mPreferenceCategoryTpInfo.addPreference(preferenceScreen);
		}

		return true;
	}

	private boolean loadCameraInfo() {
		mPreferenceCategoryCameraInfo.removeAll();

		String content = readFile(mFileCameraInfo);
		if (content == null) {
			return false;
		}

		List<PreferenceScreen> preferenceScreens = new ArrayList<PreferenceScreen>();

		for (String info : content.split("\\s*\\n\\s*")) {
			String[] map = info.split("\\s*:\\s*");
			if (map == null || map.length != 2) {
				continue;
			}

			String prefix = map[0];

			if (prefix.equals("SENSOR_MAIN")) {
				prefix = getResources().getString(R.string.back_camera);
			} else if (prefix.equals("SENSOR_SUB")) {
				prefix = getResources().getString(R.string.front_camera);
			} else {
				continue;
			}

			HashMap<String, String> hashMap = parseFile(map[1]);
			if (hashMap == null) {
				continue;
			}

			String name = hashMap.get("name");
			if (name != null) {
				PreferenceScreen preference = mPreferenceCategoryCameraInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(prefix + getResources().getString(R.string.info_ic));
				preference.setSummary(name);
				preferenceScreens.add(preference);

				HuaCameraInfo cameraInfo = HuaCameraInfo.getCameraInfo(name);
				if (cameraInfo != null) {
					preference = mPreferenceCategoryCameraInfo.getPreferenceManager().createPreferenceScreen(this);
					preference.setTitle(prefix + getResources().getString(R.string.info_ic_vendor));
					preference.setSummary(cameraInfo.getIcVendor());
					preferenceScreens.add(preference);

					preference = mPreferenceCategoryCameraInfo.getPreferenceManager().createPreferenceScreen(this);
					preference.setTitle(prefix + getResources().getString(R.string.info_module_vendor));
					preference.setSummary(cameraInfo.getVendorName());
					preferenceScreens.add(preference);
				}
			}

			String width = hashMap.get("max_width");
			String height = hashMap.get("max_height");
			if (width != null && height != null) {
				PreferenceScreen preference = mPreferenceCategoryCameraInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(prefix + getResources().getString(R.string.info_resolution));
				preference.setSummary(width + " * " + height);
				preferenceScreens.add(preference);
			}
		}

		for (PreferenceScreen preferenceScreen : preferenceScreens) {
			preferenceScreen.setKey(mPreferenceCategoryCameraInfo.getKey());
			mPreferenceCategoryCameraInfo.addPreference(preferenceScreen);
		}

		return true;
	}

	private boolean loadFlashInfo() {
		mPreferenceCategoryFlashInfo.removeAll();

		HashMap<String, String> hashMap = parseFile(mFileFlashInfo);
		if (hashMap == null) {
			return false;
		}

		List<PreferenceScreen> preferenceScreens = new ArrayList<PreferenceScreen>();

		String id = hashMap.get("id");
		if (id != null) {
			PreferenceScreen preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
			preference.setTitle(R.string.info_id);
			preference.setSummary(id.toUpperCase());
			preferenceScreens.add(preference);

			HuaFlashInfo info = HuaFlashInfo.getFlashInfo(id);
			if (info != null) {
				preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(R.string.info_ic);
				preference.setSummary(info.getIc());
				preferenceScreens.add(preference);

				preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(R.string.info_vendor);
				preference.setSummary(info.getVendorName());
				preferenceScreens.add(preference);

				preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(R.string.info_capacity);
				preference.setSummary(info.getCapacity());
				preferenceScreens.add(preference);
			}
		}

		for (PreferenceScreen preferenceScreen : preferenceScreens) {
			preferenceScreen.setKey(mPreferenceCategoryFlashInfo.getKey());
			mPreferenceCategoryFlashInfo.addPreference(preferenceScreen);
		}

		return true;
	}

	private boolean loadGsensorInfo() {
		mPreferenceCategoryGsensorInfo.removeAll();

		HuaGsensorDevice device = HuaGsensorDevice.getGsensorDevice();
		if (device == null) {
			return false;
		}

		List<PreferenceScreen> preferenceScreens = new ArrayList<PreferenceScreen>();

		PreferenceScreen preference = mPreferenceCategoryGsensorInfo.getPreferenceManager().createPreferenceScreen(this);
		preference.setTitle(R.string.info_ic);
		preference.setSummary(device.getIcName());
		preferenceScreens.add(preference);

		preference = mPreferenceCategoryGsensorInfo.getPreferenceManager().createPreferenceScreen(this);
		preference.setTitle(R.string.info_axis_count);
		preference.setSummary(Integer.toString(device.getAxisCount()));
		preferenceScreens.add(preference);

		preference = mPreferenceCategoryGsensorInfo.getPreferenceManager().createPreferenceScreen(this);
		preference.setTitle(R.string.info_dev_path);
		preference.setSummary(device.getFileDevice().getPath());
		preferenceScreens.add(preference);

		for (PreferenceScreen preferenceScreen : preferenceScreens) {
			preferenceScreen.setKey(mPreferenceCategoryGsensorInfo.getKey());
			mPreferenceCategoryGsensorInfo.addPreference(preferenceScreen);
		}

		return true;
	}

	public HuaTouchscreenDevice getTouchscreenDevice() {
		return mTouchscreenDevice;
	}
}
