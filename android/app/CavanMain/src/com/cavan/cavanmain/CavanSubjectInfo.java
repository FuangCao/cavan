package com.cavan.cavanmain;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;

import com.cavan.android.CavanDatabaseProvider;
import com.cavan.android.CavanDatabaseProvider.CavanDatabaseTable;

public class CavanSubjectInfo {

	public static final String TABLE_NAME = "subjects";
	public static final Uri CONTENT_URI = Uri.withAppendedPath(CavanNotificationProvider.CONTENT_URI, TABLE_NAME);

	public static final String KEY_QUESTION = "question";
	public static final String KEY_ANSWER = "answer";
	public static final String KEY_TIME = "time";

	private static final String[] PROJECTION = {
		CavanDatabaseProvider.KEY_ID, KEY_QUESTION, KEY_ANSWER, KEY_TIME
	};

	private long mDatabaseId;
	private String mQuestion;
	private String mAnswer;
	private long mTime;

	public CavanSubjectInfo(String question, String answer) {
		mQuestion = question;
		mAnswer = answer;
		mTime = System.currentTimeMillis();
	}

	public CavanSubjectInfo(Cursor cursor) {
		parse(cursor);
	}

	public void parse(Cursor cursor) {
		mDatabaseId = cursor.getLong(0);
		mQuestion = cursor.getString(1);
		mAnswer = cursor.getString(2);
		mTime = cursor.getLong(3);
	}

	public long getDatabaseId() {
		return mDatabaseId;
	}

	public void setDatabaseId(long id) {
		mDatabaseId = id;
	}

	public String getQuestion() {
		return mQuestion;
	}

	public void setQuestion(String question) {
		mQuestion = question;
	}

	public String getAnswer() {
		return mAnswer;
	}

	public void setAnswer(String answer) {
		mAnswer = answer;
	}

	public long getTime() {
		return mTime;
	}

	public void setTime(long time) {
		mTime = time;
	}

	public ContentValues getContentValues() {
		ContentValues values = new ContentValues();

		values.put(KEY_QUESTION, mQuestion);
		values.put(KEY_ANSWER, mAnswer);
		values.put(KEY_TIME, mTime);

		return values;
	}

	public Uri save(ContentResolver resolver) {
		return resolver.insert(CONTENT_URI, getContentValues());
	}

	public int delete(ContentResolver resolver) {
		String where = CavanDatabaseProvider.KEY_ID + '=' + mDatabaseId;
		return resolver.delete(CONTENT_URI, where, null);
	}

	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder();

		builder.append('[');
		builder.append(KEY_QUESTION).append(':').append(mQuestion).append(", ");
		builder.append(KEY_ANSWER).append(':').append(mAnswer).append(", ");
		builder.append(KEY_TIME).append(':').append(mTime);
		builder.append(']');

		return builder.toString();
	}

	public static void initDatabaseTable(CavanNotificationProvider provider) {
		CavanDatabaseTable table = provider.getTable(TABLE_NAME);

		table.setColumn(KEY_QUESTION, "text unique on conflict replace");
		table.setColumn(KEY_ANSWER, "text");
		table.setColumn(KEY_TIME, "long");
	}

	public static CavanSubjectInfo[] read(Cursor cursor) {
		CavanSubjectInfo[] users = new CavanSubjectInfo[cursor.getCount()];

		for (int i = 0; i < users.length; i++) {
			if (cursor.moveToPosition(i)) {
				users[i] = new CavanSubjectInfo(cursor);
			}
		}

		return users;
	}

	public static CavanSubjectInfo[] query(ContentResolver resolver, String selection, String[] selectionArgs) {
		Cursor cursor = resolver.query(CONTENT_URI, PROJECTION, selection, selectionArgs, null);
		if (cursor != null) {
			return read(cursor);
		}

		return null;
	}

	public static CavanSubjectInfo[] query(ContentResolver resolver) {
		return query(resolver, null, null);
	}

	public static CavanSubjectInfo get(ContentResolver resolver, String question) {
		CavanSubjectInfo[] subjects = query(resolver, KEY_QUESTION + "=?", new String[] { question });
		if (subjects != null && subjects.length > 0) {
			return subjects[0];
		}

		return null;
	}

	public static int delete(ContentResolver resolver, String question) {
		String where = KEY_QUESTION + "=?";
		String[] selectionArgs = new String[] { question };
		return resolver.delete(CONTENT_URI, where, selectionArgs);
	}
}
