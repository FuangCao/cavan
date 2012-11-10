package com.eavoo.cavan;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.telephony.SmsMessage;
import android.util.Log;

public class EavooShortMessage
{
	private static final String TAG = "EavooShortMessage";
	// EDUA# + 手机的IMEI号 + “,” + 手机软件版本号 + “,,,,#”
	private static final Pattern mPattern = Pattern.compile("^EDUA#.*,{4}#$");

	private long mDate;
	private String mAddress;
	private String mBody;

	@Override
	public String toString()
	{
		StringBuilder builder = new StringBuilder();
		builder.append("Address = " + mAddress);
		builder.append(", Body = " + mBody);
		builder.append(", Date = " + Long.toHexString(mDate));
		return builder.toString();
	}

	public EavooShortMessage(long date, String address, String body) throws Exception
	{
		checkMessageBody(body);
		mDate = date;
		mAddress = address;
		mBody = body;
	}

	public EavooShortMessage(Cursor cursor)
	{
		mDate = cursor.getLong(1);
		mAddress = cursor.getString(2);
		mBody = cursor.getString(3);
	}

	public EavooShortMessage(Object []pdus) throws Exception
	{
		StringBuilder bodyBuilder = new StringBuilder();
		SmsMessage sms = null;

		for (Object pdu : pdus)
		{
			sms = SmsMessage.createFromPdu((byte[]) pdu);
			bodyBuilder.append(sms.getMessageBody());
		}

		if (sms == null)
		{
			throw new Exception("No SmsMessage found!");
		}

		mBody = bodyBuilder.toString();
		checkMessageBody(mBody);
		mAddress = sms.getOriginatingAddress();
		mDate = sms.getTimestampMillis() / 1000;
	}

	public static boolean isEavooShortMessage(String body)
	{
		Matcher matcher = mPattern.matcher(body);
		return matcher.find();
	}

	public static void checkMessageBody(String body) throws Exception
	{
		if (isEavooShortMessage(body) == false)
		{
			Log.e(TAG, "Body = " + body);
			throw new Exception("This isn't a eavoo message!");
		}
	}

	public static byte[] convertToByteArray(int value)
	{
		byte[] bytes =
		{
			(byte) (value & 0xFF),
			(byte) ((value >> 8) & 0xFF),
			(byte) ((value >> 16) & 0xFF),
			(byte) ((value >> 24) & 0xFF),
		};

		return bytes;
	}

	public static byte[] convertToByteArray(long value)
	{
		byte[] bytes =
		{
			(byte) (value & 0xFF),
			(byte) ((value >> 8) & 0xFF),
			(byte) ((value >> 16) & 0xFF),
			(byte) ((value >> 24) & 0xFF),
		};

		return bytes;
	}

	public static void sendByteArray(OutputStream outputStream, int type, byte[] bytes) throws IOException
	{
		outputStream.write(type);
		outputStream.write(convertToByteArray(bytes.length));
		outputStream.write(bytes);
	}

	public static void sendLong(OutputStream outputStream, int type, long value) throws IOException
	{
		outputStream.write(type);
		outputStream.write(convertToByteArray(value));
	}


	public void sendTo(OutputStream outputStream) throws IOException
	{
		sendByteArray(outputStream, EavooClientSocket.SMS_TYPE_ADDRESS, mAddress.getBytes());
		sendByteArray(outputStream, EavooClientSocket.SMS_TYPE_BODY, mBody.getBytes());
		sendLong(outputStream, EavooClientSocket.SMS_TYPE_DATE, mDate);
		outputStream.write(EavooClientSocket.SMS_TYPE_END);
	}

	public byte[] toByteArray() throws IOException
	{
		ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
		sendTo(outputStream);
		return outputStream.toByteArray();
	}

	public ContentValues toContentValues()
	{
		ContentValues values = new ContentValues();
		values.put("date", mDate);
		values.put("address", mAddress);
		values.put("body", mBody);

		return values;
	}

	public static void CreateDatabaseTable(SQLiteDatabase database, String name)
	{
		StringBuilder sqlBuilder = new StringBuilder("create table " + name);
		sqlBuilder.append("(id integer primary key autoincrement, ");
		sqlBuilder.append("date long not null, ");
		sqlBuilder.append("address text not null, ");
		sqlBuilder.append("body text not null)");
		database.execSQL(sqlBuilder.toString());
	}
}
