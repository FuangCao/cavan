package com.cavan.accessibility;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

import com.cavan.android.CavanAndroid;
import com.cavan.resource.R;

public class CavanKeyguardActivity extends Activity implements OnClickListener {

	public static CavanKeyguardActivity instance;

	public static boolean show(Context context) {
		return CavanAndroid.startActivity(context, CavanKeyguardActivity.class);
	}

	private Button mButtonUnlock;

	@Override
	protected void onCreate(Bundle bundle) {
		super.onCreate(bundle);
		instance = this;

		CavanAndroid.setActivityKeyguardEnable(this, false);
		setContentView(R.layout.keyguard);

		mButtonUnlock = (Button) findViewById(R.id.buttonUnlock);
		mButtonUnlock.setOnClickListener(this);
	}

	@Override
	protected void onDestroy() {
		instance = null;
		super.onDestroy();
	}

	@Override
	protected void onResume() {
		super.onResume();
		CavanAndroid.requestDismissKeyguard(this);
	}

	@Override
	public void onClick(View v) {
		finish();
	}
}
