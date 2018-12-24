package com.cavan.cavanmain;

import java.io.File;
import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.Arrays;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.Point;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.text.format.Formatter;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageInfo;
import com.cavan.android.CavanQrCode;
import com.cavan.cavanjni.CavanJni;
import com.cavan.cavanjni.HttpService;
import com.cavan.java.CavanFile;

public class CavanShareAppActivity extends Activity implements OnItemClickListener, OnCheckedChangeListener, OnClickListener {

	private String mUrl;
	private Button mButtonCopy;
	private TextView mTextViewUrl;
	private ListView mListViewApps;
	private EditText mEditTextFilter;
	private ImageView mImageViewQrCode;
	private CheckBox mCheckBoxGameOnly;
	private CheckBox mCheckBoxShowSysApp;

	private CavanPackageInfo[] mPackageInfos = new CavanPackageInfo[0];

	private BaseAdapter mAdapterApps = new BaseAdapter() {

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			TextView view;

			if (convertView != null) {
				view = (TextView) convertView;
			} else {
				view = new TextView(CavanShareAppActivity.this);
				view.setPadding(20, 20, 20, 20);
			}

			CavanPackageInfo info = mPackageInfos[position];
			StringBuilder builder = new StringBuilder();
			long length = info.getSourceFile().length();

			builder.append(info.getApplicationName());
			builder.append(" - ").append(info.getPackageName()).append('\n');
			builder.append(Formatter.formatFileSize(CavanShareAppActivity.this, length));

			view.setText(builder);
			view.setClickable(false);

			return view;
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public Object getItem(int position) {
			return mPackageInfos[position];
		}

		@Override
		public int getCount() {
			return mPackageInfos.length;
		}

		@Override
		public void notifyDataSetChanged() {
			String filter = mEditTextFilter.getText().toString().trim();
			ArrayList<CavanPackageInfo> list = new ArrayList<CavanPackageInfo>();

			PackageManager manager = getPackageManager();

			for (PackageInfo app : manager.getInstalledPackages(0)) {
				CavanPackageInfo info = new CavanPackageInfo(manager, app);

				if (info.isSystemApp()) {
					if (!mCheckBoxShowSysApp.isChecked()) {
						continue;
					}
				} else if (mCheckBoxGameOnly.isChecked()) {
					if (!info.isGameApp()) {
						continue;
					}
				}

				if (filter.isEmpty() || info.getApplicationName().contains(filter) || info.getPackageName().contains(filter)) {
					list.add(info);
				}
			}

			CavanPackageInfo[] array = new CavanPackageInfo[list.size()];
			list.toArray(array);
			Arrays.sort(array);

			mPackageInfos = array;

			super.notifyDataSetChanged();
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_share_app);

		Intent intent = getIntent();
		mUrl = intent.getStringExtra("url");

		mButtonCopy = (Button) findViewById(R.id.buttonCopy);
		mImageViewQrCode = (ImageView) findViewById(R.id.imageViewQrCode);
		mListViewApps = (ListView) findViewById(R.id.listViewApps);
		mTextViewUrl = (TextView) findViewById(R.id.textViewUrl);
		mEditTextFilter = (EditText) findViewById(R.id.editTextFilter);
		mCheckBoxShowSysApp = (CheckBox) findViewById(R.id.checkBoxShowSysApp);
		mCheckBoxGameOnly = (CheckBox) findViewById(R.id.checkBoxGameOnly);

		mButtonCopy.setOnClickListener(this);

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
		CavanFile dir = HttpService.getSharedDir(this);
		CavanFile file = CavanJni.symlinkApk(dir, mPackageInfos[position]);

		try {
			String url = mUrl + dir.getAbsolutePath() + File.separatorChar + URLEncoder.encode(file.getName(), "UTF-8");
			// CavanAndroid.postClipboardText(this, url);
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

	@Override
	public void onClick(View v) {
		CharSequence text = mTextViewUrl.getText();
		if (text.length() > 0) {
			CavanAndroid.postClipboardText(this, text);
		}
	}
}
