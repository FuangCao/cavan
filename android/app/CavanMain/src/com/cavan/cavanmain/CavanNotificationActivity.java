package com.cavan.cavanmain;

import java.util.HashMap;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.app.Activity;
import android.content.Intent;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.text.method.LinkMovementMethod;
import android.text.util.Linkify;
import android.text.util.Linkify.TransformFilter;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;

public class CavanNotificationActivity extends Activity {

	public static final String SCHEME = "cavan://";
	public static final String ACTION_ALIPAY = "alipay";
	public static final String ACTION_OPEN = "open";

	public static final Pattern[] sAlipayPatterns = {
		Pattern.compile("\\b(\\d{8})\\b"),
		Pattern.compile("(红包|口令)\\s*[:：]\\s*(\\w+)"),
		Pattern.compile("(红包|口令)\\s+(\\w+)"),
	};

	public static final Pattern[] sRedPacketPatterns = {
		Pattern.compile("\\[(\\w+红包)\\]"),
		Pattern.compile("【(\\w+红包)】"),
	};

	private ListView mMessageView;
	private ContentObserver mContentObserver;
	private MessageAdapter mAdapter = new MessageAdapter();
	private CavanNotification mNotification = new CavanNotification();
	private HashMap<String, HashMap<String, String>> mSchemeMap = new HashMap<String, HashMap<String,String>>();

	private TransformFilter mTransformFilter = new TransformFilter() {

		@Override
		public String transformUrl(Matcher match, String url) {
			return match.group(match.groupCount());
		}
	};

	public String buildScheme(String pkgName, String action) {
		HashMap<String, String> map = mSchemeMap.get(action);
		if (map == null) {
			map = new HashMap<String, String>();
			mSchemeMap.put(action, map);
		}

		String scheme = map.get(pkgName);
		if (scheme == null) {
			scheme = SCHEME + pkgName + "/" + action + "/";
			map.put(pkgName, scheme);
		}

		return scheme;
	}

	public boolean isSelectionBottom() {
		int childs = mMessageView.getChildCount();
		if (childs > 0) {
			int position = mMessageView.getLastVisiblePosition();
			if (position + 1 < mMessageView.getCount()) {
				return false;
			} else {
				View last = mMessageView.getChildAt(childs - 1);
				if (last.getBottom() > mMessageView.getBottom()) {
					return false;
				}
			}
		}

		return true;
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		Uri uri = getIntent().getData();
		if (uri == null) {
			setContentView(R.layout.notification_activity);

			mMessageView = (ListView) findViewById(R.id.listViewMessage);
			mMessageView.setAdapter(mAdapter);
			mAdapter.updateCursor();

			mContentObserver = new MessageObserver(new Handler());
			getContentResolver().registerContentObserver(CavanNotification.CONTENT_URI, true, mContentObserver);
		} else {
			CavanAndroid.logE("uri = " + uri);

			try {
				List<String> paths = uri.getPathSegments();
				String action = paths.get(0);

				if (ACTION_ALIPAY.equals(action)) {
					String code = paths.get(1);

					RedPacketListenerService.postRedPacketCode(this, paths.get(1));
					RedPacketListenerService.startAlipayActivity(this);

					CavanAndroid.showToastLong(this, "支付宝口令: " + code);
				} else if (ACTION_OPEN.equals(action)) {
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

	public class MessageObserver extends ContentObserver {

		public MessageObserver(Handler handler) {
			super(handler);
		}

		@Override
		public void onChange(boolean selfChange) {
			mAdapter.updateCursor();
		}
	}

	public class MessageAdapter extends BaseAdapter {

		private Cursor mCursor;

		public void setCursor(Cursor cursor) {
			int count;

			if (cursor != null) {
				count = cursor.getCount();
			} else {
				count = 0;
			}

			String title = getResources().getString(R.string.text_message_count);
			setTitle(title + count);

			boolean isBottom = isSelectionBottom();

			mCursor = cursor;
			notifyDataSetChanged();

			if (isBottom) {
				mMessageView.setSelection(count - 1);
			}
		}

		public void updateCursor() {
			Cursor cursor = CavanNotification.queryAll(getContentResolver(), CavanNotification.KEY_TIMESTAMP);
			setCursor(cursor);
		}

		@Override
		public int getCount() {
			if (mCursor == null) {
				return 0;
			}

			return mCursor.getCount();
		}

		@Override
		public Object getItem(int position) {
			return null;
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public View getView(int position, View view, ViewGroup viewGroup) {
			if (!mCursor.moveToPosition(position)) {
				return null;
			}

			if (!mNotification.parse(mCursor)) {
				return null;
			}

			if (view == null) {
				view = View.inflate(CavanNotificationActivity.this, R.layout.message_item, null);
			}

			TextView viewTitle = (TextView) view.findViewById(R.id.textViewTitle);
			viewTitle.setText(mNotification.buildTitle());

			TextView viewContent = (TextView) view.findViewById(R.id.textViewContent);
			viewContent.setText(mNotification.getContent());

			Linkify.addLinks(viewContent, Linkify.WEB_URLS);

			String pkgName = mNotification.getPackageName();
			String scheme = buildScheme(pkgName, ACTION_ALIPAY);

			for (Pattern pattern : sAlipayPatterns) {
				Linkify.addLinks(viewContent, pattern, scheme, null, mTransformFilter);
			}

			scheme = buildScheme(pkgName, ACTION_OPEN);

			for (Pattern pattern : sRedPacketPatterns) {
				Linkify.addLinks(viewContent, pattern, scheme, null, mTransformFilter);
			}

			viewContent.setMovementMethod(LinkMovementMethod.getInstance());

			return view;
		}
	}
}
