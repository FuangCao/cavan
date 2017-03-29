package com.cavan.cavanmain;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

public class RedPacketEditActivity extends Activity {

	public static Intent getIntent(Context context) {
		return new Intent(context, RedPacketEditActivity.class);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.red_packet_editor);
	}
}
