package com.eavoo.cavan;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;

import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.telephony.SmsMessage;

public class EavooShortMessage
{
	@Override
	public String toString()
	{
		StringBuilder builder = new StringBuilder();
		builder.append("Phone = " + mPhone);
		builder.append(", Content = " + mContent);
		builder.append(", Date = " + Long.toHexString(mDate));
		return builder.toString();
	}

	private long mDate;
	private String mPhone;
	private String mContent;

	public EavooShortMessage(long date, String phone, String content)
	{
		mDate = date;
		mPhone = phone;
		mContent = content;
	}

	public EavooShortMessage(Cursor cursor)
	{
		mDate = cursor.getLong(1);
		mPhone = cursor.getString(2);
		mContent = cursor.getString(3);
	}

	public EavooShortMessage(Object []pdus) throws Exception
	{
		StringBuilder contentBuilder = new StringBuilder();
		SmsMessage sms = null;

		for (Object pdu : pdus)
		{
			sms = SmsMessage.createFromPdu((byte[]) pdu);
			contentBuilder.append(sms.getMessageBody());
		}

		if (sms == null)
		{
			throw new Exception();
		}

		mPhone = sms.getOriginatingAddress();
		mDate = sms.getTimestampMillis() / 1000;
		mContent = contentBuilder.toString();
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
		sendByteArray(outputStream, EavooClientSocket.SMS_TYPE_PHONE, mPhone.getBytes());
		sendByteArray(outputStream, EavooClientSocket.SMS_TYPE_CONTENT, mContent.getBytes());
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
		values.put("phone", mPhone);
		values.put("content", mContent);

		return values;
	}

	public static void CreateDatabaseTable(SQLiteDatabase database, String name)
	{
		StringBuilder sqlBuilder = new StringBuilder("create table " + name);
		sqlBuilder.append("(id integer primary key autoincrement, ");
		sqlBuilder.append("date long not null, ");
		sqlBuilder.append("phone text not null, ");
		sqlBuilder.append("content text not null)");
		database.execSQL(sqlBuilder.toString());
	}
}
