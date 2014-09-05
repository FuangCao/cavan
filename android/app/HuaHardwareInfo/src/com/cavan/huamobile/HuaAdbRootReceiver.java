package com.cavan.huamobile;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class HuaAdbRootReceiver extends BroadcastReceiver {
	@Override
	public void onReceive(Context context, Intent intent) {
		intent = new Intent(Intent.ACTION_MAIN);
		intent.setFlags(intent.FLAG_ACTIVITY_NEW_TASK);
		intent.setClass(context, HuaAdbRootActivity.class);
		context.startActivity(intent);
	}
}
