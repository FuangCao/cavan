package com.cavan.wifi;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import android.Manifest;
import android.app.Activity;
import android.os.Bundle;
import android.os.storage.StorageManager;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanStorageVolume;
import com.cavan.cavanmain.R;

public class WifiPasswordActivity extends Activity implements OnItemSelectedListener {

	private StorageManager mManager;
	private Spinner mSpinnerConfigFiles;
	private ListView mListViewAccessPoints;
	private WifiConfigFile mCurrFile;

	private BaseAdapter mAdapterConfigFiles = new BaseAdapter() {

		List<WifiConfigFile> mFiles = new ArrayList<WifiConfigFile>();

		@Override
		public void notifyDataSetChanged() {
			mFiles = findWifiConfigFiles();
			super.notifyDataSetChanged();
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			TextView view;

			if (convertView == null) {
				view = new TextView(WifiPasswordActivity.this);
				view.setTextSize(8);
				view.setPadding(0, 30, 0, 30);
			} else {
				view = (TextView) convertView;
			}

			view.setText(mFiles.get(position).getAbsolutePath());

			return view;
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public Object getItem(int position) {
			return mFiles.get(position);
		}

		@Override
		public int getCount() {
			return mFiles.size();
		}
	};

	private BaseAdapter mAdapterAccessPoints = new BaseAdapter() {

		private ArrayList<String> mAccessPoints = new ArrayList<String>();

		@Override
		public void notifyDataSetChanged() {
			if (mCurrFile != null) {
				mAccessPoints = mCurrFile.getAccessPoints();
			}

			super.notifyDataSetChanged();
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			TextView view;

			if (convertView == null) {
				view = new TextView(WifiPasswordActivity.this);
				view.setTextSize(8);
			} else {
				view = (TextView) convertView;
			}

			view.setText(mAccessPoints.get(position));

			return view;
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public Object getItem(int position) {
			return mAccessPoints.get(position);
		}

		@Override
		public int getCount() {
			return mAccessPoints.size();
		}
	};

	public void findWifiConfigFiles(List<WifiConfigFile> files, File dir) {
		for (String name : dir.list()) {
			if ("wifiConfig.db".equals(name)) {
				files.add(new WifiConfigFile(dir, name));
			} else {
				File file = new File(dir, name);
				if (file.isDirectory()) {
					findWifiConfigFiles(files, file);
				}
			}
		}
	}

	public List<WifiConfigFile> findWifiConfigFiles() {
		List<WifiConfigFile> files = new ArrayList<WifiConfigFile>();

		ArrayList<CavanStorageVolume> volumes = CavanStorageVolume.getVolumeListMounted(mManager);
		if (volumes == null) {
			return files;
		}

		for (CavanStorageVolume volume : volumes) {
			File root = volume.getPathFile();
			if (root == null) {
				continue;
			}

			root = new File(root, "HuaweiBackup");
			if (!root.isDirectory()) {
				continue;
			}

			for (String name : root.list()) {
				if (name.startsWith("backupFiles")) {
					findWifiConfigFiles(files, new File(root, name));
				}
			}
		}

		return files;
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_wifi_password);

		mManager = (StorageManager) getSystemService(STORAGE_SERVICE);
		mSpinnerConfigFiles = (Spinner) findViewById(R.id.spinnerConfigFiles);
		mListViewAccessPoints = (ListView) findViewById(R.id.listViewAccessPoints);

		CavanAndroid.checkAndRequestPermissions(this, Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE);

		mListViewAccessPoints.setAdapter(mAdapterAccessPoints);

		mSpinnerConfigFiles.setOnItemSelectedListener(this);
		mSpinnerConfigFiles.setAdapter(mAdapterConfigFiles);
		mAdapterConfigFiles.notifyDataSetChanged();
	}

	@Override
	public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
		mCurrFile = (WifiConfigFile) mAdapterConfigFiles.getItem(position);
		mAdapterAccessPoints.notifyDataSetChanged();
	}

	@Override
	public void onNothingSelected(AdapterView<?> parent) {
		CavanAndroid.pLog();
	}
}
