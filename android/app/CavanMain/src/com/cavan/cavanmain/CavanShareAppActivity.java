package com.cavan.cavanmain;

import java.io.File;
import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.graphics.Bitmap;
import android.graphics.Point;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanQrCode;
import com.cavan.cavanjni.CavanJni;
import com.cavan.java.CavanFile;

public class CavanShareAppActivity extends Activity implements OnItemClickListener, OnCheckedChangeListener {

	private String mUrl;
	private TextView mTextViewUrl;
	private ListView mListViewApps;
	private EditText mEditTextFilter;
	private ImageView mImageViewQrCode;
	private CheckBox mCheckBoxGameOnly;
	private CheckBox mCheckBoxShowSysApp;

	private List<PackageInfo> mPackageInfos = new ArrayList<PackageInfo>();

	private BaseAdapter mAdapterApps = new BaseAdapter() {

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			TextView view;

			if (convertView != null) {
				view = (TextView) convertView;
			} else {
				view = new TextView(CavanShareAppActivity.this);
			}

			view.setText(getAppName(mPackageInfos.get(position)));
			view.setClickable(false);

			return view;
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public Object getItem(int position) {
			return mPackageInfos.get(position);
		}

		@Override
		public int getCount() {
			return mPackageInfos.size();
		}

		@Override
		public void notifyDataSetChanged() {
			String filter = mEditTextFilter.getText().toString().trim();
			ArrayList<PackageInfo> list = new ArrayList<PackageInfo>();

			for (PackageInfo info : getPackageManager().getInstalledPackages(0)) {
				int flags = info.applicationInfo.flags;

				if (!mCheckBoxShowSysApp.isChecked() && (flags & ApplicationInfo.FLAG_SYSTEM) != 0) {
					continue;
				}

				if (mCheckBoxGameOnly.isChecked() && (flags & ApplicationInfo.FLAG_IS_GAME) == 0) {
					continue;
				}

				if (filter.isEmpty() || getAppName(info).contains(filter)) {
					list.add(info);
				}
			}

			mPackageInfos = list;

			super.notifyDataSetChanged();
		}
	};

	private String getAppName(PackageInfo info) {
		CharSequence label = getPackageManager().getApplicationLabel(info.applicationInfo);
		if (label != null) {
			return label.toString();
		}

		return info.packageName;
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_share_app);

		Intent intent = getIntent();
		mUrl = intent.getStringExtra("url");

		mImageViewQrCode = (ImageView) findViewById(R.id.imageViewQrCode);
		mListViewApps = (ListView) findViewById(R.id.listViewApps);
		mTextViewUrl = (TextView) findViewById(R.id.textViewUrl);
		mEditTextFilter = (EditText) findViewById(R.id.editTextFilter);
		mCheckBoxShowSysApp = (CheckBox) findViewById(R.id.checkBoxShowSysApp);
		mCheckBoxGameOnly = (CheckBox) findViewById(R.id.checkBoxGameOnly);

		mListViewApps.setOnItemClickListener(this);
		mListViewApps.setAdapter(mAdapterApps);
		mAdapterApps.notifyDataSetChanged();

		mEditTextFilter.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {
				mAdapterApps.notifyDataSetChanged();
			}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {
			}

			@Override
			public void afterTextChanged(Editable s) {
			}
		});

		mCheckBoxShowSysApp.setOnCheckedChangeListener(this);
		mCheckBoxGameOnly.setOnCheckedChangeListener(this);
	}

	@Override
	public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
		PackageInfo info = mPackageInfos.get(position);
		String name = CavanFile.replaceInvalidFilenameChar(getAppName(info), '_') + ".apk";
		CavanFile dir = new CavanFile(getCacheDir(), "apk");

		if (dir.mkdirSafe()) {
			String path = dir.getAbsolutePath() + File.separatorChar + name;
			CavanJni.symlink(info.applicationInfo.sourceDir, path);
		}

		try {
			String url = mUrl + dir.getAbsolutePath() + File.separatorChar + URLEncoder.encode(name, "UTF-8");
			CavanAndroid.postClipboardText(this, url);
			mTextViewUrl.setText(url);

			Point point = new Point();
			view.getDisplay().getSize(point);
			int width = Math.min(point.x, point.y) / 2;

			Bitmap bitmap = CavanQrCode.encodeBitmap(url, width, width);

			if (bitmap != null) {
				mImageViewQrCode.setImageBitmap(bitmap);
				mImageViewQrCode.setVisibility(View.VISIBLE);
			}
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
		mAdapterApps.notifyDataSetChanged();
	}
}
