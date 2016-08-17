package com.cavan.cavanmain;

import java.util.Date;
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
import android.text.util.Linkify;
import android.text.util.Linkify.TransformFilter;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

public class CavanNotificationActivity extends Activity {

	public static final String SCHEME = "cavan";

	public static final String AUTHORITY_RED_PACKET = "red_packet";
	public static final String SCHEME_RED_PACKET = SCHEME + "://" + AUTHORITY_RED_PACKET + "/";

	public static final String AUTHORITY_ALIPAY = "alipay";
	public static final String SCHEME_ALIPAY = SCHEME + "://" + AUTHORITY_ALIPAY + "/";

	public static final HashMap<String, String> sRedPacketAppMap = new HashMap<String, String>();

	public static final Pattern[] sAlipayPatterns = {
		Pattern.compile("\\b(\\d{8})\\b"),
		Pattern.compile("(红包|口令)\\s*[:：]\\s*(\\w+)"),
		Pattern.compile("(红包|口令)\\s+(\\w+)"),
	};

	public static final Pattern[] sRedPacketPatterns = {
		Pattern.compile("\\[(\\w+)红包\\]"),
		Pattern.compile("【(\\w+)红包】"),
	};

	private static final String[] PROJECTION = {
		CavanNotification.KEY_TIMESTAMP,
		CavanNotification.KEY_TITLE,
		CavanNotification.KEY_USER_NAME,
		CavanNotification.KEY_GROUP_NAME,
		CavanNotification.KEY_CONTENT,
	};

	static {
		sRedPacketAppMap.put("QQ", "com.tencent.mobileqq");
		sRedPacketAppMap.put("微信", "com.tencent.mm");
	}

	private ListView mMessageView;
	private ContentObserver mContentObserver;
	private MessageAdapter mAdapter = new MessageAdapter();

	private TransformFilter mTransformFilter = new TransformFilter() {

		@Override
		public String transformUrl(Matcher match, String url) {
			return match.group(match.groupCount());
		}
	};

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
			List<String> paths = uri.getPathSegments();
			if (paths.size() > 0) {
				String authority = uri.getAuthority();
				if (AUTHORITY_ALIPAY.equals(authority)) {
					RedPacketListenerService.postRedPacketCode(this, paths.get(0));
					RedPacketListenerService.startAlipayActivity(this);
				} else if (AUTHORITY_RED_PACKET.equals(authority)) {
					String pkgName = sRedPacketAppMap.get(paths.get(0));
					if (pkgName != null) {
						Intent intent = getPackageManager().getLaunchIntentForPackage(pkgName);
						if (intent != null) {
							intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
							startActivity(intent);
						}
					}
				}
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

			boolean atLast = mMessageView.getLastVisiblePosition() + 1 >= mMessageView.getCount();

			mCursor = cursor;
			notifyDataSetChanged();

			if (atLast) {
				mMessageView.setSelection(count - 1);
			}
		}

		public void updateCursor() {
			Cursor cursor = CavanNotification.queryAll(getContentResolver(), PROJECTION, CavanNotification.KEY_TIMESTAMP);
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

		@SuppressWarnings("deprecation")
		@Override
		public View getView(int position, View convertView, ViewGroup viewGroup) {
			if (!mCursor.moveToPosition(position)) {
				return null;
			}

			long time = mCursor.getLong(0);
			String title = mCursor.getString(1);
			String user = mCursor.getString(2);
			String group = mCursor.getString(3);
			String content = mCursor.getString(4);

			View view;

			if (convertView != null) {
				view = convertView;
			} else {
				view = View.inflate(CavanNotificationActivity.this, R.layout.message_item, null);
			}

			TextView viewTitle = (TextView) view.findViewById(R.id.textViewTitle);
			TextView viewContent = (TextView) view.findViewById(R.id.textViewContent);

			StringBuilder builder = new StringBuilder(new Date(time).toLocaleString());

			if (user != null) {
				builder.append("\n");
				builder.append(user);

				if (group != null) {
					builder.append("@");
					builder.append(group);
				}
			} else if (group != null) {
				builder.append("\n");
				builder.append(group);
			} else if (title != null) {
				builder.append("\n");
				builder.append(title);
			}

			viewTitle.setText(builder.toString());
			viewContent.setText(content);

			Linkify.addLinks(viewContent, Linkify.WEB_URLS);

			for (Pattern pattern : sAlipayPatterns) {
				Linkify.addLinks(viewContent, pattern, SCHEME_ALIPAY, null, mTransformFilter);
			}

			for (Pattern pattern : sRedPacketPatterns) {
				Linkify.addLinks(viewContent, pattern, SCHEME_RED_PACKET, null, mTransformFilter);
			}

			return view;
		}
	}
}
