package com.cavan.huahardwareinfo;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.HashMap;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.util.Log;

public class HuaHardwareInfoActivity extends PreferenceActivity {
	private static final String TAG = "Cavan";

	private File mFileLcdInfo = new File("/sys/devices/platform/sprdfb.0/dev_info");
	private File mFileCameraInfo = new File("/sys/devices/platform/sc8800g_dcam.0/dev_info");
	private File mFileFlashInfo = new File("/sys/devices/platform/sprd_nand/dev_info");

	private PreferenceCategory mPreferenceCategoryLcdInfo;
	private PreferenceCategory mPreferenceCategoryTpInfo;
	private PreferenceCategory mPreferenceCategoryCameraInfo;
	private PreferenceCategory mPreferenceCategoryFlashInfo;
	private PreferenceCategory mPreferenceCategoryGsensorInfo;

	private OnPreferenceClickListener mClickListenerLcdInfo = new OnPreferenceClickListener() {
		@Override
		public boolean onPreferenceClick(Preference preference) {
			loadLcdInfo();
			return false;
		}
	};

	private OnPreferenceClickListener mClickListenerTpInfo = new OnPreferenceClickListener() {
		@Override
		public boolean onPreferenceClick(Preference preference) {
			loadTpInfo();
			return false;
		}
	};

	private OnPreferenceClickListener mClickListenerCameraInfo = new OnPreferenceClickListener() {
		@Override
		public boolean onPreferenceClick(Preference preference) {
			loadCameraInfo();
			return false;
		}
	};

	private OnPreferenceClickListener mClickListenerFlashInfo = new OnPreferenceClickListener() {
		@Override
		public boolean onPreferenceClick(Preference preference) {
			loadFlashInfo();
			return false;
		}
	};

	private OnPreferenceClickListener mClickListenerGsensorInfo = new OnPreferenceClickListener() {
		@Override
		public boolean onPreferenceClick(Preference preference) {
			loadGsensorInfo();
			return false;
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.hua_hardware_info);

		mPreferenceCategoryLcdInfo = (PreferenceCategory) findPreference("lcd_info");
		mPreferenceCategoryTpInfo = (PreferenceCategory) findPreference("tp_info");
		mPreferenceCategoryCameraInfo = (PreferenceCategory) findPreference("camera_info");
		mPreferenceCategoryFlashInfo = (PreferenceCategory) findPreference("flash_info");
		mPreferenceCategoryGsensorInfo = (PreferenceCategory) findPreference("gsensor_info");

		loadLcdInfo();
		loadTpInfo();
		loadCameraInfo();
		loadFlashInfo();
		loadGsensorInfo();
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

	private boolean loadLcdInfo() {
		mPreferenceCategoryLcdInfo.removeAll();

		HashMap<String, String> hashMap = parseFile(mFileLcdInfo);
		if (hashMap == null) {
			return false;
		}

		String id = hashMap.get("id");
		if (id != null) {
			PreferenceScreen preference = mPreferenceCategoryLcdInfo.getPreferenceManager().createPreferenceScreen(this);;
			preference.setTitle(R.string.info_id);
			preference.setSummary(id);
			preference.setOnPreferenceClickListener(mClickListenerLcdInfo);
			mPreferenceCategoryLcdInfo.addPreference(preference);

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
				preference.setOnPreferenceClickListener(mClickListenerLcdInfo);
				mPreferenceCategoryLcdInfo.addPreference(preference);
			}

			HuaLcdVendorInfo vendorInfo = lcdInfo.getVendorInfo();
			if (vendorInfo != null) {
				preference = mPreferenceCategoryLcdInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(R.string.info_module_vendor);
				preference.setSummary(vendorInfo.getVendorName());
				preference.setOnPreferenceClickListener(mClickListenerLcdInfo);
				mPreferenceCategoryLcdInfo.addPreference(preference);
			}
		}


		String width = hashMap.get("width");
		String height = hashMap.get("height");
		if (width != null && height != null) {
			PreferenceScreen preference = mPreferenceCategoryLcdInfo.getPreferenceManager().createPreferenceScreen(this);;
			preference.setTitle(R.string.info_resolution);
			preference.setSummary(width + " * " + height);
			preference.setOnPreferenceClickListener(mClickListenerLcdInfo);
			mPreferenceCategoryLcdInfo.addPreference(preference);
		}

		String bus_width = hashMap.get("bus_width");
		if (bus_width != null) {
			PreferenceScreen preference = mPreferenceCategoryLcdInfo.getPreferenceManager().createPreferenceScreen(this);;
			preference.setTitle(R.string.info_bus_width);
			preference.setSummary(bus_width);
			preference.setOnPreferenceClickListener(mClickListenerLcdInfo);
			mPreferenceCategoryLcdInfo.addPreference(preference);
		}

		return true;
	}

	private boolean loadTpInfo() {
		mPreferenceCategoryTpInfo.removeAll();

		HuaTouchscreenDevice device = HuaTouchscreenDevice.getTouchscreenDevice();
		if (device == null) {
			return false;
		}

		PreferenceScreen preference = mPreferenceCategoryTpInfo.getPreferenceManager().createPreferenceScreen(this);
		preference.setTitle(R.string.info_ic);
		preference.setSummary(device.getIcName());
		preference.setOnPreferenceClickListener(mClickListenerTpInfo);
		mPreferenceCategoryTpInfo.addPreference(preference);

		preference = mPreferenceCategoryTpInfo.getPreferenceManager().createPreferenceScreen(this);
		preference.setTitle(R.string.info_fw_id);
		preference.setSummary(Integer.toString(device.getFwId(), 16));
		preference.setOnPreferenceClickListener(mClickListenerTpInfo);
		mPreferenceCategoryTpInfo.addPreference(preference);

		HuaTouchScreenVendorInfo info = device.getVendorInfo();
		if (info != null) {
			preference = mPreferenceCategoryTpInfo.getPreferenceManager().createPreferenceScreen(this);
			preference.setTitle(R.string.info_module_vendor);
			preference.setSummary(info.getVendorName());
			preference.setOnPreferenceClickListener(mClickListenerTpInfo);
			mPreferenceCategoryTpInfo.addPreference(preference);
		}

		String fwName = device.getFwName();
		if (fwName != null) {
			preference = mPreferenceCategoryTpInfo.getPreferenceManager().createPreferenceScreen(this);
			preference.setTitle(R.string.info_fw_name);
			preference.setSummary(fwName);
			preference.setOnPreferenceClickListener(mClickListenerTpInfo);
			mPreferenceCategoryTpInfo.addPreference(preference);
		}

		preference = mPreferenceCategoryTpInfo.getPreferenceManager().createPreferenceScreen(this);
		preference.setTitle(R.string.info_dev_path);
		preference.setSummary(device.getFileDevice().getPath());
		preference.setOnPreferenceClickListener(mClickListenerTpInfo);
		mPreferenceCategoryTpInfo.addPreference(preference);

		return true;
	}

	private boolean loadCameraInfo() {
		mPreferenceCategoryCameraInfo.removeAll();

		String content = readFile(mFileCameraInfo);
		if (content == null) {
			return false;
		}

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
				preference.setOnPreferenceClickListener(mClickListenerCameraInfo);
				mPreferenceCategoryCameraInfo.addPreference(preference);

				HuaCameraInfo cameraInfo = HuaCameraInfo.getCameraInfo(name);
				if (cameraInfo != null) {
					preference = mPreferenceCategoryCameraInfo.getPreferenceManager().createPreferenceScreen(this);
					preference.setTitle(prefix + getResources().getString(R.string.info_ic_vendor));
					preference.setSummary(cameraInfo.getIcVendor());
					preference.setOnPreferenceClickListener(mClickListenerCameraInfo);
					mPreferenceCategoryCameraInfo.addPreference(preference);

					preference = mPreferenceCategoryCameraInfo.getPreferenceManager().createPreferenceScreen(this);
					preference.setTitle(prefix + getResources().getString(R.string.info_module_vendor));
					preference.setSummary(cameraInfo.getVendorName());
					preference.setOnPreferenceClickListener(mClickListenerCameraInfo);
					mPreferenceCategoryCameraInfo.addPreference(preference);
				}
			}

			String width = hashMap.get("max_width");
			String height = hashMap.get("max_height");
			if (width != null && height != null) {
				PreferenceScreen preference = mPreferenceCategoryCameraInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(prefix + getResources().getString(R.string.info_resolution));
				preference.setSummary(width + " * " + height);
				preference.setOnPreferenceClickListener(mClickListenerCameraInfo);
				mPreferenceCategoryCameraInfo.addPreference(preference);
			}
		}

		return true;
	}

	private boolean loadFlashInfo() {
		mPreferenceCategoryFlashInfo.removeAll();

		HashMap<String, String> hashMap = parseFile(mFileFlashInfo);
		if (hashMap == null) {
			return false;
		}

		String id = hashMap.get("id");
		if (id != null) {
			PreferenceScreen preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
			preference.setTitle(R.string.info_id);
			preference.setSummary(id.toUpperCase());
			preference.setOnPreferenceClickListener(mClickListenerFlashInfo);
			mPreferenceCategoryFlashInfo.addPreference(preference);

			HuaFlashInfo info = HuaFlashInfo.getFlashInfo(id);
			if (info != null) {
				preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(R.string.info_ic);
				preference.setSummary(info.getIc());
				preference.setOnPreferenceClickListener(mClickListenerFlashInfo);
				mPreferenceCategoryFlashInfo.addPreference(preference);

				preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(R.string.info_vendor);
				preference.setSummary(info.getVendorName());
				preference.setOnPreferenceClickListener(mClickListenerFlashInfo);
				mPreferenceCategoryFlashInfo.addPreference(preference);

				preference = mPreferenceCategoryFlashInfo.getPreferenceManager().createPreferenceScreen(this);
				preference.setTitle(R.string.info_capacity);
				preference.setSummary(info.getCapacity());
				preference.setOnPreferenceClickListener(mClickListenerFlashInfo);
				mPreferenceCategoryFlashInfo.addPreference(preference);
			}
		}

		return true;
	}

	private boolean loadGsensorInfo() {
		mPreferenceCategoryGsensorInfo.removeAll();

		HuaGsensorDevice device = HuaGsensorDevice.getGsensorDevice();
		if (device == null) {
			return false;
		}

		PreferenceScreen preference = mPreferenceCategoryGsensorInfo.getPreferenceManager().createPreferenceScreen(this);
		preference.setTitle(R.string.info_ic);
		preference.setSummary(device.getIcName());
		preference.setOnPreferenceClickListener(mClickListenerGsensorInfo);
		mPreferenceCategoryGsensorInfo.addPreference(preference);

		preference = mPreferenceCategoryGsensorInfo.getPreferenceManager().createPreferenceScreen(this);
		preference.setTitle(R.string.info_axis_count);
		preference.setSummary(Integer.toString(device.getAxisCount()));
		preference.setOnPreferenceClickListener(mClickListenerGsensorInfo);
		mPreferenceCategoryGsensorInfo.addPreference(preference);

		preference = mPreferenceCategoryGsensorInfo.getPreferenceManager().createPreferenceScreen(this);
		preference.setTitle(R.string.info_dev_path);
		preference.setSummary(device.getFileDevice().getPath());
		preference.setOnPreferenceClickListener(mClickListenerGsensorInfo);
		mPreferenceCategoryGsensorInfo.addPreference(preference);

		return true;
	}
}
