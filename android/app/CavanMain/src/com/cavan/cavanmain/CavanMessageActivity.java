package com.cavan.cavanmain;

import java.util.List;

import android.app.Activity;
import android.content.Intent;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.widget.ListView;

import com.cavan.android.CavanAndroid;

public class CavanMessageActivity extends Activity {

	private ListView mMessageView;
	private CavanMessageAdapter mAdapter;
	private ContentObserver mContentObserver = new ContentObserver(new Handler()) {

		@Override
		public void onChange(boolean selfChange) {
			updateData();
		}
	};

	private void updateData() {
		Cursor cursor = mAdapter.updateAll();
		if (cursor != null) {
			String title = getResources().getString(R.string.text_message_count);
			setTitle(title + cursor.getCount());
		}
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		Uri uri = getIntent().getData();
		if (uri == null) {
			setContentView(R.layout.notification_activity);

			mMessageView = (ListView) findViewById(R.id.listViewMessage);
			mAdapter = new CavanMessageAdapter(mMessageView);
			updateData();

			getContentResolver().registerContentObserver(CavanNotification.CONTENT_URI, true, mContentObserver);
		} else {
			CavanAndroid.eLog("uri = " + uri);

			try {
				List<String> paths = uri.getPathSegments();
				String action = paths.get(0);

				if (CavanMessageView.ACTION_ALIPAY.equals(action)) {
					String code = paths.get(1);

					RedPacketListenerService.postRedPacketCode(this, paths.get(1));
					RedPacketListenerService.startAlipayActivity(this);

					CavanAndroid.showToastLong(this, "支付宝口令: " + code);
				} else if (CavanMessageView.ACTION_OPEN.equals(action)) {
					Intent intent = getPackageManager().getLaunchIntentForPackage(uri.getHost());
					if (intent != null) {
						intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
						startActivity(intent);
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			}

			finish();
		}
	}

	@Override
	protected void onDestroy() {
		if (mContentObserver != null) {
			getContentResolver().unregisterContentObserver(mContentObserver);
		}

		super.onDestroy();
	}
}
