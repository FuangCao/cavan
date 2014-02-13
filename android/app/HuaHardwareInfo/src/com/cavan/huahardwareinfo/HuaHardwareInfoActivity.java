package com.cavan.huahardwareinfo;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.os.Build;
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

	private static File mFileLcdInfo;
	private static File mFileCameraInfo;
	private static File mFileFlashInfo;
	private static File mFileFlashSize;

	private PreferenceCategory mPreferenceCategoryLcdInfo;
	private PreferenceCategory mPreferenceCategoryTpInfo;
	private PreferenceCategory mPreferenceCategoryCameraInfo;
	private PreferenceCategory mPreferenceCategoryFlashInfo;
	private PreferenceCategory mPreferenceCategoryGsensorInfo;
	private HuaTouchscreenDevice mTouchscreenDevice;

	static {
		if (Build.HARDWARE.equals("sp8810")) {
			mFileLcdInfo = new File("/sys/devices/platform/sprdfb.0/dev_info");
			mFileCameraInfo = new File("/sys/devices/platform/sc8800g_dcam.0/dev_info");
			mFileFlashInfo = new File("/sys/devices/platform/sprd_nand/dev_info");
		} else {
			mFileLcdInfo = new File("/sys/devices/virtual/graphics/fb0/dev_info");
			mFileCameraInfo = new File("/sys/class/video4linux/v4l-subdev4/dev_info");
			mFileFlashInfo = new File("/sys/bus/mmc/devices/mmc0:0001");
			mFileFlashSize = new File(mFileFlashInfo, "block/mmcblk0/size");
		}
	}

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
		if (file == null || file.exists() == false) {
			return null;
		}

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

		return content.trim();
	}

	private HashMap<String, String> parseFile(String content) {
		HashMap<String, String> hashMap = new HashMap<String, String>();

		for (String item : content.split("\\s*,\\s+")) {
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

		HuaLcdInfo lcdInfo;
		PreferenceScreen preference;
		List<PreferenceScreen> preferenceScreens = new ArrayList<PreferenceScreen>();

		String id = hashMap.get("id");
		if (id != null) {
			preference = mPreferenceCategoryLcdInfo.getPreferenceManager().createPreferenceScreen(this);;
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

			lcdInfo = new HuaLcdInfo(Integer.parseInt(id, 16), vendorId);
		} else {
			String name = hashMap.get("name");
			if (name != null) {
				preference = mPreferenceCategoryLcdInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(R.string.info_name);
				preference.setSummary(name);
				preferenceScreens.add(preference);

				lcdInfo = new HuaLcdInfo(name);
			} else {
				lcdInfo = null;
			}
		}

		if (lcdInfo != null) {
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

		String type = hashMap.get("type");
		if (type != null) {
			preference = mPreferenceCategoryLcdInfo.getPreferenceManager().createPreferenceScreen(this);;
			preference.setTitle(R.string.info_type);
			preference.setSummary(type);
			preferenceScreens.add(preference);
		}

		String width = hashMap.get("width");
		String height = hashMap.get("height");
		if (width != null && height != null) {
			preference = mPreferenceCategoryLcdInfo.getPreferenceManager().createPreferenceScreen(this);;
			preference.setTitle(R.string.info_resolution);
			preference.setSummary(width + " * " + height);
			preferenceScreens.add(preference);
		}

		String bus_width = hashMap.get("bus_width");
		if (bus_width != null) {
			preference = mPreferenceCategoryLcdInfo.getPreferenceManager().createPreferenceScreen(this);;
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
		} else {
			mTouchscreenDevice.fillVendorInfo();
		}

		List<PreferenceScreen> preferenceScreens = new ArrayList<PreferenceScreen>();

		PreferenceScreen preference = mPreferenceCategoryTpInfo.getPreferenceManager().createPreferenceScreen(this);
		preference.setTitle(R.string.info_ic);
		preference.setSummary(mTouchscreenDevice.getIcName());
		preferenceScreens.add(preference);

		HuaTouchscreenVendorInfo info = mTouchscreenDevice.getVendorInfo();
		if (info != null) {
			preference = mPreferenceCategoryTpInfo.getPreferenceManager().createPreferenceScreen(this);
			preference.setTitle(R.string.info_fw_id);
			preference.setSummary(info.toString());
			preferenceScreens.add(preference);

			preference = mPreferenceCategoryTpInfo.getPreferenceManager().createPreferenceScreen(this);
			preference.setTitle(R.string.info_module_vendor);
			preference.setSummary(info.getVendorName());
			preferenceScreens.add(preference);
		}

		String fwName = mTouchscreenDevice.getFwName();
		if (fwName != null) {
			preference = mPreferenceCategoryTpInfo.getPreferenceManager().createPreferenceScreen(this);
			preference.setTitle(R.string.info_fw_upgrade);
			preference.setOnPreferenceClickListener(new OnPreferenceClickListener() {
				@Override
				public boolean onPreferenceClick(Preference preference) {
					HuaTpUpgradeDialog dialog = new HuaTpUpgradeDialog(HuaHardwareInfoActivity.this);
					dialog.show();
					return true;
				}
			});
			preference.setSummary(fwName);
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
			String[] map = info.split("\\s*:\\s+");
			if (map == null) {
				continue;
			}

			String prefix;
			String description;

			if (map.length > 1) {
				prefix = map[0];

				if (prefix.equals("SENSOR_MAIN")) {
					prefix = getResources().getString(R.string.back_camera);
				} else if (prefix.equals("SENSOR_SUB")) {
					prefix = getResources().getString(R.string.front_camera);
				} else {
					continue;
				}

				description = map[1];
			} else {
				description = map[0];
				prefix = "";
			}

			HashMap<String, String> hashMap = parseFile(description);
			if (hashMap == null) {
				continue;
			}

			String name = hashMap.get("name");
			if (name != null) {
				PreferenceScreen preference = mPreferenceCategoryCameraInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(prefix + getResources().getString(R.string.info_ic));
				preference.setSummary(name.toUpperCase());
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

	private boolean loadNandInfo(List<PreferenceScreen> preferenceScreens) {
		HashMap<String, String> hashMap = parseFile(mFileFlashInfo);
		if (hashMap == null) {
			return false;
		}

		String id = hashMap.get("id");
		if (id == null) {
			return false;
		}

		PreferenceScreen preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
		preference.setTitle(R.string.info_id);
		preference.setSummary(id.toLowerCase());
		preferenceScreens.add(preference);

		HuaNandFlashInfo info = HuaNandFlashInfo.getFlashInfo(id);
		if (info != null) {
			String ic = info.getIc();
			if (ic != null) {
				preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(R.string.info_ic);
				preference.setSummary(ic);
				preferenceScreens.add(preference);
			}

			preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
			preference.setTitle(R.string.info_vendor);
			preference.setSummary(info.getVendor());
			preferenceScreens.add(preference);

			NandFlashDevice device = info.getFlashDevice();
			if (device != null) {
				preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(R.string.info_capacity);
				preference.setSummary(device.getChipSizeString());
				preferenceScreens.add(preference);

				preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(R.string.info_detail);
				preference.setSummary(device.getName());
				preferenceScreens.add(preference);
			}
		}

		return true;
	}

	private boolean loadEmmcInfo(List<PreferenceScreen> preferenceScreens) {
		String content = readFile(new File(mFileFlashInfo, "name"));
		if (content != null) {
			PreferenceScreen preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
			preference.setTitle(R.string.info_name);
			preference.setSummary(content);
			preferenceScreens.add(preference);
		}

		content = readFile(new File(mFileFlashInfo, "type"));
		if (content != null) {
			PreferenceScreen preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
			preference.setTitle(R.string.info_type);
			preference.setSummary(content);
			preferenceScreens.add(preference);
		}

		content = readFile(new File(mFileFlashInfo, "manfid"));
		if (content != null) {
			int manfind = Integer.parseInt(content.substring(2), 16);
			PreferenceScreen preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
			preference.setTitle(R.string.info_id);
			preference.setSummary(String.format("%02x", manfind));
			preferenceScreens.add(preference);

			Integer vendor = HuaMmcInfo.getVendorNameById(manfind);
			if (vendor != null) {
				preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(R.string.info_vendor);
				preference.setSummary(vendor);
				preferenceScreens.add(preference);
			}
		}

		content = readFile(mFileFlashSize);
		if (content != null) {
			PreferenceScreen preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
			preference.setTitle(R.string.info_capacity);
			double capicity = Integer.parseInt(content);
			preference.setSummary(String.format("%.2f Gib", capicity * 512 / 1024 / 1024 / 1024));
			preferenceScreens.add(preference);
		}

		return true;
	}

	private boolean loadFlashInfo() {
		mPreferenceCategoryFlashInfo.removeAll();

		List<PreferenceScreen> preferenceScreens = new ArrayList<PreferenceScreen>();

		if (mFileFlashInfo.isDirectory()) {
			if (loadEmmcInfo(preferenceScreens) == false) {
				return false;
			}
		} else {
			if (loadNandInfo(preferenceScreens) == false) {
				return false;
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
