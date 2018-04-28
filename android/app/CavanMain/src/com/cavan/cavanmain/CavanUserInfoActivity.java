package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanCheckBox;
import com.cavan.android.CavanPackageName;

public class CavanUserInfoActivity extends Activity implements OnClickListener, OnItemSelectedListener, OnCheckedChangeListener {

	private static final String[] PACKAGES = {
		CavanPackageName.MM,
		CavanPackageName.QQ,
		CavanPackageName.ALIPAY,
	};

	private CavanCheckBox mCheckBoxSelectAll;
	private ListView mListViewUsers;
	private Spinner mSpinnerPackage;
	private EditText mEditTextUser;
	private EditText mEditTextPass;
	private Button mButtonAdd;
	private Button mButtonRm;

	private PackageManager mPackageManager;
	private CavanUserInfo[] mUserInfos;

	private BaseAdapter mAdapterUsers = new BaseAdapter() {

		@Override
		public void notifyDataSetChanged() {
			ApplicationInfo ainfo = (ApplicationInfo) mSpinnerPackage.getSelectedItem();
			if (ainfo == null) {
				mUserInfos = null;
			} else {
				CavanUserInfo[] infos = CavanUserInfo.query(getContentResolver(), ainfo.packageName, null);
				if (infos != null) {
					for (CavanUserInfo info : infos) {
						info.setChecked(mCheckBoxSelectAll.isChecked());
					}
				}

				mUserInfos = infos;
			}

			super.notifyDataSetChanged();
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			CavanUserInfo info = mUserInfos[position];
			UserInfoCheckbox view;

			if (convertView != null) {
				view = (UserInfoCheckbox) convertView;
			} else {
				view = new UserInfoCheckbox(CavanUserInfoActivity.this);
			}

			view.setUserInfo(info);
			view.setText(info.getAccount());
			view.setCheckedSilent(info.isChecked());
			view.setOnCheckedChangeListener(CavanUserInfoActivity.this);

			return view;
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public Object getItem(int position) {
			return mUserInfos[position];
		}

		@Override
		public int getCount() {
			if (mUserInfos == null) {
				return 0;
			}

			return mUserInfos.length;
		}
	};

	private BaseAdapter mAdapterPkgs = new BaseAdapter() {

		List<ApplicationInfo> mApplicationInfos;

		@Override
		public void notifyDataSetChanged() {
			ArrayList<ApplicationInfo> infos = new ArrayList<ApplicationInfo>();

			for (String pkg : PACKAGES) {
				try {
					ApplicationInfo info = mPackageManager.getApplicationInfo(pkg, 0);
					if (info != null) {
						infos.add(info);
					}
				} catch (NameNotFoundException e) {
					e.printStackTrace();
				}
			}

			mApplicationInfos = infos;

			super.notifyDataSetChanged();
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			TextView view;

			if (convertView != null) {
				view = (TextView) convertView;
			} else {
				view = new TextView(CavanUserInfoActivity.this);
				view.setPadding(0, 20, 0, 20);
			}

			ApplicationInfo info = mApplicationInfos.get(position);
			view.setText(mPackageManager.getApplicationLabel(info));

			return view;
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public Object getItem(int position) {
			return mApplicationInfos.get(position);
		}

		@Override
		public int getCount() {
			if (mApplicationInfos == null) {
				return 0;
			}

			return mApplicationInfos.size();
		}
	};

	public boolean isSelectedAll() {
		if (mUserInfos == null) {
			return false;
		}

		for (CavanUserInfo info : mUserInfos) {
			if (!info.isChecked()) {
				return false;
			}
		}

		return true;
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.user_info);

		mPackageManager = getPackageManager();

		mCheckBoxSelectAll = (CavanCheckBox) findViewById(R.id.checkBoxSelectAll);
		mCheckBoxSelectAll.setOnCheckedChangeListener(this);

		mSpinnerPackage = (Spinner) findViewById(R.id.spinnerPackage);
		mSpinnerPackage.setOnItemSelectedListener(this);
		mSpinnerPackage.setAdapter(mAdapterPkgs);
		mAdapterPkgs.notifyDataSetChanged();

		mListViewUsers = (ListView) findViewById(R.id.listViewUsers);
		mListViewUsers.setAdapter(mAdapterUsers);
		mAdapterUsers.notifyDataSetChanged();

		mEditTextUser = (EditText) findViewById(R.id.editTextUserName);
		mEditTextPass = (EditText) findViewById(R.id.editTextPassword);

		mButtonAdd = (Button) findViewById(R.id.buttonAdd);
		mButtonAdd.setOnClickListener(this);

		mButtonRm = (Button) findViewById(R.id.buttonRemove);
		mButtonRm.setOnClickListener(this);
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.buttonAdd:
			ApplicationInfo ainfo = (ApplicationInfo) mSpinnerPackage.getSelectedItem();
			if (ainfo == null) {
				CavanAndroid.showToast(CavanUserInfoActivity.this, "Please select application!");
				break;
			}

			String username = mEditTextUser.getText().toString().trim();
			if (username.isEmpty()) {
				CavanAndroid.showToast(CavanUserInfoActivity.this, "Please input the username!");
				break;
			}

			String password = mEditTextPass.getText().toString();
			if (password.isEmpty()) {
				CavanAndroid.showToast(CavanUserInfoActivity.this, "Please input the password!");
				break;
			}

			CavanUserInfo user = new CavanUserInfo(ainfo.packageName, username, password);
			if (user.save(getContentResolver()) == null) {
				CavanAndroid.showToast(CavanUserInfoActivity.this, R.string.add_failed);
			} else {
				CavanAndroid.showToast(CavanUserInfoActivity.this, R.string.add_success);
				mAdapterUsers.notifyDataSetChanged();
			}
			break;

		case R.id.buttonRemove:
			if (mUserInfos != null) {
				boolean changed = false;

				for (CavanUserInfo info : mUserInfos) {
					if (info.isChecked() && info.delete(getContentResolver()) > 0) {
						changed = true;
					}
				}

				if (changed) {
					mAdapterUsers.notifyDataSetChanged();
				}
			}
			break;

		default:
			CavanAndroid.eLog("Invalid view id!");
		}
	}

	@Override
	public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
		mAdapterUsers.notifyDataSetChanged();
	}

	@Override
	public void onNothingSelected(AdapterView<?> parent) {
		CavanAndroid.dLog("onNothingSelected");
	}

	@Override
	public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
		if (buttonView == mCheckBoxSelectAll) {
			mAdapterUsers.notifyDataSetChanged();
		} else {
			UserInfoCheckbox view = (UserInfoCheckbox) buttonView;
			view.getUserInfo().setChecked(isChecked);
			mCheckBoxSelectAll.setCheckedSilent(isChecked && isSelectedAll());
		}
	}

	class UserInfoCheckbox extends CavanCheckBox {

		private CavanUserInfo mUserInfo;

		public UserInfoCheckbox(Context context) {
			super(context);
		}

		public CavanUserInfo getUserInfo() {
			return mUserInfo;
		}

		public void setUserInfo(CavanUserInfo userInfo) {
			mUserInfo = userInfo;
		}
	}
}
