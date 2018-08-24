package com.cavan.qrcode;

import android.app.AlertDialog;
import android.content.Context;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

import com.cavan.android.CavanAndroid;
import com.cavan.cavanmain.R;

public class WifiDialog extends AlertDialog implements OnClickListener {

	private View mView;
	private String mSsid;
	private String mType;
	private String mPass;

	private EditText mEditTextSsid;
	private EditText mEditTextType;
	private EditText mEditTextPass;
	private Button mButtonConnect;
	private Button mButtonCancel;

	public WifiDialog(Context context, String ssid, String type, String pass) {
		super(context);
		mSsid = ssid;
		mType = type;
		mPass = pass;
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		mView = getLayoutInflater().inflate(R.layout.wifi_dialog, null);
		setView(mView);
		setInverseBackgroundForced(true);
		super.onCreate(savedInstanceState);

		mEditTextSsid = (EditText) findViewById(R.id.editTextSsid);
		mEditTextSsid.setText(mSsid);

		mEditTextType = (EditText) findViewById(R.id.editTextType);
		mEditTextType.setText(mType);

		mEditTextPass = (EditText) findViewById(R.id.editTextPass);
		mEditTextPass.setText(mPass);

		mButtonConnect = (Button) findViewById(R.id.buttonConnect);
		mButtonConnect.setOnClickListener(this);

		mButtonCancel = (Button) findViewById(R.id.buttonCancel);
		mButtonCancel.setOnClickListener(this);
	}

	@Override
	public void onClick(View v) {
		if (v == mButtonConnect) {
			String ssid = mEditTextSsid.getText().toString();

			if (ssid.length() > 0) {
				WifiConfiguration config = CavanAndroid.createWifiConfiguration(ssid);
				String type = mEditTextType.getText().toString();
				String pass = mEditTextPass.getText().toString();

				if (type.contains("WPA")) {
					CavanAndroid.initWifiConfigurationWpa(config, pass);
				} else if (type.contains("WEP")) {
					CavanAndroid.initWifiConfigurationWep(config, pass);
				} else {
					CavanAndroid.initWifiConfigurationNone(config);
				}

				try {
					WifiManager manager = (WifiManager) getContext().getSystemService(Context.WIFI_SERVICE);
					if (manager != null) {
						int id = manager.addNetwork(config);
						CavanAndroid.dLog("id = " + id);

						if (id < 0) {
							CavanAndroid.showToast(getContext(), R.string.add_ssid_failed);
						} else if (manager.enableNetwork(id, true) && manager.reconnect()) {
							CavanAndroid.showToast(getContext(), R.string.connect_success);
						} else {
							CavanAndroid.showToast(getContext(), R.string.connect_failed);
						}
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		}

		dismiss();
	}
}
