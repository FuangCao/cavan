package com.eavoo.cavan;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

import android.app.Service;
import android.content.ContentResolver;
import android.content.Intent;
import android.database.ContentObserver;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.net.Uri;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;

public class AdbSmsTranslatorService extends Service
{
	private static final int MAX_SERVICE_COUNT = 10;
	private static final String TAG = "AdbSmsTranslatorService";

	private static final String DEFAULT_DB_NAME = "eavoo.db";
	private static final String DEFAULT_TABLE_NAME = "message";

	private static final Uri SMS_CONTENT_URI = Uri.parse("content://sms");
	private static final Uri ICC_CONTENT_URI = Uri.parse("content://sms/icc");
	private static final Uri THREAD_CONTENT_URI = Uri.parse("content://mms-sms/conversations");

	public static final String ACTION_SERVICE_RUNNING = "cavan.intent.action.SERVICE_RUNNING";
	public static final String ACTION_SERVICE_STOPPED = "cavan.intent.action.SERVICE_STOPPED";
	public static final String ACTION_SERVICE_START_FAILED = "cavan.intent.action.SERVICE_START_FAILED";
	public static final String ACTION_SERVICE_STOP_FAILED = "cavan.intent.action.SERVICE_STOP_FAILED";

	// public static final String SMS_RECEIVED_ACTION = "android.provider.Telephony.SMS_RECEIVED";
	// public static final String DATA_SMS_RECEIVED_ACTION = "android.intent.action.DATA_SMS_RECEIVED";

	private ServerSocket mServerSocket;
	private int mPort;
	private EavooClientSocket[] mClientSockets = new EavooClientSocket[MAX_SERVICE_COUNT];
	private SocketLitenThread mLitenThread;
	private SQLiteOpenHelperMessage mSqLiteOpenHelperMessage;
	private SQLiteDatabase mSqLiteDatabase;
	private ContentObserverMessage mContentObservers;

	public long DatabaseInsert(EavooShortMessage message)
	{
		Log.i(TAG, "DatabaseInsert: " + message);

		return mSqLiteDatabase.insert(mSqLiteOpenHelperMessage.getTableName(), null, message.toContentValues());
	}

	public int DatabaseDelete(String[] args)
	{
		return mSqLiteDatabase.delete(mSqLiteOpenHelperMessage.getTableName(), "id=?", args);
	}

	public int DatabaseDelete(String id)
	{
		String[] args = new String[] {id};
		return DatabaseDelete(args);
	}

	public Cursor DatabaseQuertAll()
	{
		return mSqLiteDatabase.query(mSqLiteOpenHelperMessage.getTableName(), null, null, null, null, null, null);
	}

	@Override
	public IBinder onBind(Intent arg0)
	{
		Log.i(TAG, "onBind");
		return null;
	}

	@Override
	public void onCreate()
	{
		Log.i(TAG, "onCreate");
		mSqLiteOpenHelperMessage = new SQLiteOpenHelperMessage(this, DEFAULT_DB_NAME, DEFAULT_TABLE_NAME);
		mSqLiteDatabase = mSqLiteOpenHelperMessage.getWritableDatabase();
		mContentObservers = new ContentObserverMessage(getContentResolver(), new Handler());

		super.onCreate();
	}

	@Override
	public void onDestroy()
	{
		Log.i(TAG, "onDestroy");

		SocketLitenThread thread = mLitenThread;
		mLitenThread = null;

		if (mServerSocket != null)
		{
			try
			{
				mServerSocket.close();
			}
			catch (IOException e)
			{
				e.printStackTrace();

				Intent intent = new Intent(ACTION_SERVICE_STOP_FAILED);
				sendBroadcast(intent);
				mLitenThread = thread;

				return;
			}
		}

		if (thread != null)
		{
			try
			{
				thread.join();
			}
			catch (InterruptedException e)
			{
				e.printStackTrace();

				Intent intent = new Intent(ACTION_SERVICE_STOP_FAILED);
				sendBroadcast(intent);
				mLitenThread = thread;

				return;
			}
		}

		mSqLiteDatabase.close();
		super.onDestroy();
	}

	@Override
	public void onStart(Intent intent, int startId)
	{
		Log.i(TAG, "onStart");

		mPort = intent.getIntExtra("translator_port", 8888);
		if (mServerSocket != null && mServerSocket.getLocalPort() != mPort)
		{
			try
			{
				mServerSocket.close();
			}
			catch (IOException e)
			{
				e.printStackTrace();
			}
		}

		if (mLitenThread == null)
		{
			mLitenThread = new SocketLitenThread();
			mLitenThread.start();
		}

		super.onStart(intent, startId);
	}

	synchronized private int sendShortMessage(EavooShortMessage message)
	{
		byte[] buff;
		try
		{
			buff = message.toByteArray();
		}
		catch (IOException e)
		{
			e.printStackTrace();
			return -1;
		}

		int count = 0;

		for (int i = mClientSockets.length - 1; i >= 0; i--)
		{
			if (mClientSockets[i] == null)
			{
				continue;
			}

			int ret = mClientSockets[i].write(buff);
			if (ret < 0)
			{
				mClientSockets[i].close();
				mClientSockets[i] = null;
			}
			else
			{
				count++;
			}
		}

		if (count > 0)
		{
			Log.i(TAG, "sendShortMessage: " + message);
		}

		return count;
	}

	private boolean sendDatabaseToClients()
	{
		Cursor cursor = DatabaseQuertAll();
		if (cursor == null || cursor.moveToFirst() == false)
		{
			Log.i(TAG, "sendDatabaseToClients: cursor is empty!");
			return true;
		}

		String[] args = new String[1];

		do
		{
			EavooShortMessage message = new EavooShortMessage(cursor);
			if (sendShortMessage(message) > 0)
			{
				args[0] = cursor.getString(0);
				DatabaseDelete(args);
				Log.i(TAG, "Success SendDatabaseToClient: " + message);
			}
			else
			{
				Log.i(TAG, "Failed SendDatabaseToClient: " + message);
				break;
			}
		}
		while (cursor.moveToNext());

		return true;
	}

	class SocketLitenThread extends Thread
	{
		private boolean HandleRequest(Socket socket)
		{
			int index = EavooClientSocket.findFreeSocket(mClientSockets);
			if (index < 0)
			{
				Log.e(TAG, "No free socket or outputstream found!");
				return false;
			}

			Log.i(TAG, "RemoteSocketAddress = " + socket.getRemoteSocketAddress());
			Log.i(TAG, "index = " + index);

			try
			{
				mClientSockets[index] = new EavooClientSocket(socket);
			}
			catch (IOException e)
			{
				e.printStackTrace();
				mClientSockets[index] = null;
			}

			return true;
		}

		Socket accept()
		{
			while (true)
			{
				if (mLitenThread == null)
				{
					break;
				}

				if (mServerSocket == null)
				{
					try
					{
						mServerSocket = new ServerSocket(mPort);
					}
					catch (IOException e)
					{
						e.printStackTrace();

						Intent intent = new Intent(ACTION_SERVICE_START_FAILED);
						sendBroadcast(intent);

						mServerSocket = null;
						break;
					}

					Intent intent = new Intent(ACTION_SERVICE_RUNNING);
					sendBroadcast(intent);

					Log.i(TAG, "Bind to port `" + mPort + "' success");
				}

				try
				{
					return mServerSocket.accept();
				}
				catch (IOException e)
				{
					e.printStackTrace();
				}

				try
				{
					mServerSocket.close();
				}
				catch (IOException e)
				{
					e.printStackTrace();
				}

				mServerSocket = null;
			}

			return null;
		}

		@Override
		public void run()
		{
			mContentObservers.registerContentObserver();

			while (true)
			{
				Log.i(TAG, "Service ready");

				Socket socket = accept();
				if (socket == null)
				{
					break;
				}

				if (HandleRequest(socket) == false)
				{
					try
					{
						socket.close();
					}
					catch (IOException e)
					{
						e.printStackTrace();
					}
				}
				else
				{
					sendDatabaseToClients();
				}
			}

			mContentObservers.unregisterContentObserver();
			EavooClientSocket.closeAll(mClientSockets);

			Log.i(TAG, "Service exit!");

			Intent intent = new Intent(ACTION_SERVICE_STOPPED);
			sendBroadcast(intent);

			mLitenThread = null;
			mServerSocket = null;
		}
	}

	class ContentObserverMessage extends ContentObserver
	{
		private ContentResolver mContentResolver;
		private Thread mThread;

		public ContentObserverMessage(ContentResolver resolver, Handler handler)
		{
			super(handler);

			mContentResolver = resolver;
		}

		public void registerContentObserver()
		{
			receiveMessagesFromSms();
			receiveMessagesFromIcc();
			deleteAll(THREAD_CONTENT_URI);
			mContentResolver.registerContentObserver(SMS_CONTENT_URI, true, this);

			mThread = new Thread()
			{
				@Override
				public void run()
				{
					while (mThread != null)
					{
						try
						{
							sleep(5 * 60 * 1000);
						}
						catch (InterruptedException e)
						{
							e.printStackTrace();
						}

						receiveMessagesFromIcc();
					}

					mThread = null;
					Log.i(TAG, "ContentObserverMessage: thread exit!");
				}
			};

			mThread.start();
		}

		public void unregisterContentObserver()
		{
			Log.i(TAG, "unregisterContentObserver: Uri = " + SMS_CONTENT_URI);

			mContentResolver.unregisterContentObserver(this);

			if (mThread != null)
			{
				mThread.stop();
				mThread = null;
			}

			// receiveMessagesFromIcc();
		}

		private boolean receiveMessagesFromSms()
		{
			return receiveMessages(SMS_CONTENT_URI, "_id");
		}

		private boolean receiveMessagesFromIcc()
		{
			return receiveMessages(ICC_CONTENT_URI, "index_on_icc");
		}

		synchronized private boolean receiveMessages(Uri uriQuery, String keyName)
		{
			Log.i(TAG, "receiveMessages: uriQuery = " + uriQuery + ", keyName = " + keyName);

			Cursor cursor = mContentResolver.query(uriQuery, null, null, null, null);
			if (cursor == null || cursor.moveToFirst() == false)
			{
				Log.i(TAG, "receiveMessage: cursor is empty!");
				return false;
			}

			int indexKey = cursor.getColumnIndex(keyName);
			int indexDate = cursor.getColumnIndex("date");
			int indexAddress = cursor.getColumnIndex("address");
			int indexBody = cursor.getColumnIndex("body");

			if (indexKey < 0 || indexDate < 0 || indexAddress < 0 || indexBody < 0)
			{
				StringBuilder builder = new StringBuilder();
				builder.append("indexKey = " + indexKey);
				builder.append(", indexDate = " + indexDate);
				builder.append(", indexAddress = " + indexAddress);
				builder.append(", indexBody = " + indexBody);
				Log.e(TAG, "onChange: " + builder);
				return false;
			}

			while (true)
			{
				long date = cursor.getLong(indexDate) / 1000;
				String address = cursor.getString(indexAddress);
				String body = cursor.getString(indexBody);

				EavooShortMessage message;
				try
				{
					message = new EavooShortMessage(date, address, body);
				}
				catch (Exception e)
				{
					message = null;
				}

				if (message == null || (sendShortMessage(message) > 0 || DatabaseInsert(message) >= 0))
				{
					Uri delUri = uriQuery.buildUpon().appendPath(cursor.getString(indexKey)).build();
					int delCount = mContentResolver.delete(delUri, null, null);
					Log.i(TAG, "Delete uri = " + delUri + ", count = " + delCount);
				}

				if (cursor.moveToNext() == false)
				{
					break;
				}
			}

			return true;
		}

		private int deleteAll(Uri uri)
		{
			Log.i(TAG, "delUri = " + uri);
			int count = mContentResolver.delete(uri, null, null);
			Log.i(TAG, "delete count = " + count);

			return count;
		}

		@Override
		public void onChange(boolean selfChange)
		{
			Log.i(TAG, "onChange: selfChange = " + selfChange);

			if (selfChange || receiveMessagesFromSms() || receiveMessagesFromIcc())
			{
				Log.i(TAG, "receiveMessages success");
			}
			else
			{
				deleteAll(THREAD_CONTENT_URI);
			}
		}
	}
}