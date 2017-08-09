package com.cavan.bluetoothinterphone;

import android.app.Activity;
import android.media.AudioManager;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class MainActivity extends Activity implements OnClickListener {

	private AudioManager mManager;

	private Button mButtonStop;
	private Button mButtonStart;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mButtonStop = (Button) findViewById(R.id.buttonStop);
		mButtonStop.setOnClickListener(this);

		mButtonStart = (Button) findViewById(R.id.buttonStart);
		mButtonStart.setOnClickListener(this);

		mManager = (AudioManager) getSystemService(AUDIO_SERVICE);
	}

	@Override
	public void onClick(View v) {
		if (v == mButtonStart) {
			mManager.setBluetoothScoOn(true);
			mManager.startBluetoothSco();
			mManager.setMode(AudioManager.MODE_IN_CALL);
		} else if (v == mButtonStop) {
			mManager.setMode(AudioManager.MODE_NORMAL);
			mManager.stopBluetoothSco();
			mManager.setBluetoothScoOn(false);
		}
	}
}
