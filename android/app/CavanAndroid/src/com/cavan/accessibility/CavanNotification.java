package com.cavan.accessibility;

import java.text.DateFormat;
import java.util.Date;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.os.Bundle;
import android.service.notification.StatusBarNotification;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanDatabaseProvider;
import com.cavan.android.CavanDatabaseProvider.CavanDatabaseTable;
import com.cavan.java.CavanString;
import com.cavan.java.RedPacketFinder;

public class CavanNotification extends CavanRedPacket {

	public static final String TABLE_NAME = "notification";
	public static final String KEY_TIMESTAMP = "timestamp";
	public static final String KEY_PACKAGE = "package";
	public static final String KEY_TITLE = "title";
	public static final String KEY_USER_NAME = "user_name";
	public static final String KEY_GROUP_NAME = "group_name";
	public static final String KEY_CONTENT = "content";

	public static final String[] PROJECTION = {
		KEY_TIMESTAMP, KEY_PACKAGE, KEY_TITLE, KEY_USER_NAME, KEY_GROUP_NAME, KEY_CONTENT
	};

	public static void initDatabaseTable(CavanDatabaseProvider provider) {
		CavanDatabaseTable table = provider.getTable(TABLE_NAME);

		table.setColumn(KEY_TIMESTAMP, "long");
		table.setColumn(KEY_PACKAGE, "text");
		table.setColumn(KEY_TITLE, "text");
		table.setColumn(KEY_USER_NAME, "text");
		table.setColumn(KEY_GROUP_NAME, "text");
		table.setColumn(KEY_CONTENT, "text");
	}

	protected Notification mNotification;
	protected long mTimestamp;

	protected String mTitle;
	protected String mContent;
	protected String mUserName;
	protected String mGroupName;
	protected String mPackageName;
	protected RedPacketFinder mFinder = new RedPacketFinder();

	public CavanNotification() {
		updateTimestamp();
	}

	public CavanNotification(Notification notification) {
		updateTimestamp();
		mNotification = notification;

		CharSequence title = notification.extras.getCharSequence(Notification.EXTRA_TITLE);
		if (title != null) {
			mTitle = title.toString();
		}

		CavanAndroid.dLog("[" + mTitle + "] ================================================================================");

		CharSequence text = notification.tickerText;
		if (text == null) {
			text = notification.extras.getCharSequence(Notification.EXTRA_TEXT);
		}

		if (text != null) {
			String content = text.toString();
			CavanAndroid.dLog(content);
			parse(content);
			mFinder.split(mContent);
		}
	}

	public CavanNotification(StatusBarNotification notification) {
		this(notification.getNotification());
		mPackageName = notification.getPackageName();
	}

	public CavanNotification(Cursor cursor) throws Exception {
		updateTimestamp();

		if (!parse(cursor)) {
			throw new Exception("Failed to parse cursor");
		}
	}

	public CavanNotification(String pkgName, String user, String content, boolean parse)
	{
		updateTimestamp();
		mPackageName = pkgName;
		mUserName = user;
		mContent = content;

		if (parse) {
			parse(content);
		}

		mFinder.split(mContent);
	}

	public Notification getNotification() {
		return mNotification;
	}

	public Bundle getExtras() {
		if (mNotification == null) {
			return null;
		}

		return mNotification.extras;
	}

	public String getTitle() {
		return mTitle;
	}

	public String getContent() {
		return mContent;
	}

	public String getUserName() {
		if (mUserName == null) {
			return "unknown";
		}

		return mUserName;
	}

	public String getGroupName() {
		return mGroupName;
	}

	public String getName() {
		if (mGroupName != null) {
			return mGroupName;
		}

		if (mUserName != null) {
			return mUserName;
		}

		return null;
	}

	public String getApplicationName(Context context) {
		String packageName = getPackageName();
		if (packageName == null) {
			return null;
		}

		return CavanAndroid.getApplicationLabel(context, packageName);
	}

	public String getUserDescription(Context context) {
		if (mGroupName != null) {
			return mGroupName;
		}

		if (mUserName != null) {
			return mUserName;
		}

		String name = getApplicationName(context);
		if (name != null) {
			return name;
		}

		return "未知用户";
	}

	public long getCodeDelay() {
		if (mContent != null) {
			if (mContent.contains("手气王")) {
				return 15000;
			}

			if (mContent.contains("运气王")) {
				return 5000;
			}
		}

		return 0;
	}

	public RedPacketFinder getFinder() {
		return mFinder;
	}

	public void setNotification(Notification notification) {
		mNotification = notification;
	}

	public void setTitle(String title) {
		mTitle = title;
	}

	public void setContent(String content) {
		mContent = content;
	}

	public void setUserName(String userName) {
		mUserName = userName;
	}

	public void setGroupName(String groupName) {
		mGroupName = groupName;
	}

	public String getPackageName() {
		if (mPackageName != null) {
			return mPackageName;
		}

		if (mPackage != null) {
			return mPackage.getName();
		}

		return "unknown";
	}

	public void setPackageName(String packageName) {
		mPackageName = packageName;
	}

	public long getTimestamp() {
		return mTimestamp;
	}

	public void setTimestamp(long timestamp) {
		mTimestamp = timestamp;
	}

	public void updateTimestamp() {
		mTimestamp = System.currentTimeMillis();
	}

	protected boolean isRedPacket(String prefix) {
		if (mContent == null) {
			return false;
		}

		if (!mContent.startsWith(prefix)) {
			return false;
		}

		if (mContent.contains("测") || mContent.contains("挂")) {
			return false;
		}

		if (CavanString.getLineCount(mContent) > 1) {
			return false;
		}

		return true;
	}

	public boolean isRedPacket() {
		return false;
	}

	public boolean sendPendingIntent() {
		if (mNotification == null || mSendTimes > 0) {
			return false;
		}

		PendingIntent intent = mNotification.contentIntent;
		if (intent == null) {
			return false;
		}

		try {
			intent.send();
			return true;
		} catch (CanceledException e) {
			e.printStackTrace();
		}

		return false;
	}

	public void parse(String content) {
        int end = content.indexOf(':');
        if (end > 0) {
            int start = end - 1;

            for (start = end - 1; start >= 0; start--) {
                if (content.charAt(start) == '-') {
                    mGroupName = content.substring(0, start);
                    break;
                }
            }

            mUserName = content.substring(start + 1, end);
            mContent = content.substring(end + 1);
        }
	}

	public boolean parse(Cursor cursor) {
		try {
			int column = 0;

			mTimestamp = cursor.getLong(column++);
			mPackageName = cursor.getString(column++);
			mTitle = cursor.getString(column++);
			mUserName = cursor.getString(column++);
			mGroupName = cursor.getString(column++);
			mContent = cursor.getString(column++);

			return true;
		} catch (Exception e) {
			return false;
		}
	}

	public ContentValues getContentValues() {
		ContentValues values = new ContentValues(6);

		values.put(KEY_TIMESTAMP, mTimestamp);

		if (mPackageName != null) {
			values.put(KEY_PACKAGE, mPackageName.toString());
		}

		if (mTitle != null) {
			values.put(KEY_TITLE, mTitle.toString());
		}

		if (mUserName != null) {
			values.put(KEY_USER_NAME, mUserName.toString());
		}

		if (mGroupName != null) {
			values.put(KEY_GROUP_NAME, mGroupName.toString());
		}

		if (mContent != null) {
			values.put(KEY_CONTENT, mContent.toString());
		}

		return values;
	}

	public String buildTitle() {
		DateFormat format = DateFormat.getDateTimeInstance();
		StringBuilder builder = new StringBuilder(format.format(new Date(mTimestamp)));

		if (mUserName != null) {
			builder.append("\n");
			builder.append(mUserName);

			if (mGroupName != null) {
				builder.append("@");
				builder.append(mGroupName);
			}
		} else if (mGroupName != null) {
			builder.append("\n");
			builder.append(mGroupName);
		} else if (mTitle != null) {
			builder.append("\n");
			builder.append(mTitle);
		}

		return builder.toString();
	}

	public boolean addRedPacket(String user) {
		if (!isRedPacket()) {
			return false;
		}

		CavanAccessibilityPackage pkg = getPackage();
		if (pkg != null) {
			if (isExcluded(pkg.getService(), user)) {
				CavanAndroid.dLog("Exclude user: " + user);
			} else {
				pkg.addPacket(this);
			}
		}

		return true;
	}

	public CavanRedPacketAlipay getRedPacketAlipay(String code) {
		return CavanRedPacketAlipay.get(code, true, false);
	}

	@Override
	public String getPacketName() {
		return "通知";
	}

	@Override
	public synchronized boolean launch() {
		if (sendPendingIntent()) {
			mSendTimes++;
			return true;
		}

		return super.launch();
	}

	@Override
	public boolean equals(Object o) {
		if (super.equals(o)) {
			return true;
		}

		if (o instanceof CavanNotification) {
			CavanNotification notification = (CavanNotification) o;
			return getUserName().equals(notification.getUserName());
		}

		if (o instanceof String) {
			return getUserName().equals(o);
		}

		return false;
	}
}
