package com.eavoo.cavan;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.net.Uri;
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
	public static final Uri CONTENT_CONVERSATIONS_URI = Uri.parse("content://mms-sms/conversations");

	public static final String ACTION_SERVICE_RUNNING = "cavan.intent.action.SERVICE_RUNNING";
	public static final String ACTION_SERVICE_STOPPED = "cavan.intent.action.SERVICE_STOPPED";
	public static final String ACTION_SERVICE_START_FAILED = "cavan.intent.action.SERVICE_START_FAILED";
	public static final String ACTION_SERVICE_STOP_FAILED = "cavan.intent.action.SERVICE_STOP_FAILED";

	public static final String SMS_RECEIVED_ACTION = "android.provider.Telephony.SMS_RECEIVED";
	public static final String DATA_SMS_RECEIVED_ACTION = "android.intent.action.DATA_SMS_RECEIVED";

	private ServerSocket mServerSocket;
	private EavooClientSocket[] mClientSockets = new EavooClientSocket[MAX_SERVICE_COUNT];
	private boolean mServiceShouldStop = true;
	private SocketLitenThread mLitenThread;
	private SQLiteOpenHelperMessage mSqLiteOpenHelperMessage;
	private SQLiteDatabase mSqLiteDatabase;

	private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver()
	{
		@Override
		public void onReceive(Context context, Intent intent)
		{
			receiveToDatabase();
		}
	};

	private boolean setTranslatorPort(int port) throws IOException
	{
		if (mServerSocket != null)
		{
			if (mServerSocket.getLocalPort() == port)
			{
				return false;
			}

			mServerSocket.close();
		}

		Log.i(TAG, String.format("Server bind to port `%d'", port));

		mServerSocket = new ServerSocket(port);

		return true;
	}

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

		super.onCreate();
	}

	@Override
	public void onDestroy()
	{
		Log.i(TAG, "onDestroy");

		mServiceShouldStop = true;
		if (mServerSocket != null)
		{
			try
			{
				mServerSocket.close();
				mServerSocket = null;
			}
			catch (IOException e)
			{
				mServiceShouldStop = false;
				Intent intent = new Intent(ACTION_SERVICE_STOP_FAILED);
				sendBroadcast(intent);
				e.printStackTrace();
				return;
			}
		}

		if (mLitenThread != null)
		{
			try
			{
				mLitenThread.join();
			}
			catch (InterruptedException e)
			{
				Intent intent = new Intent(ACTION_SERVICE_STOP_FAILED);
				sendBroadcast(intent);
				e.printStackTrace();
				return;
			}

			mLitenThread = null;
		}

		mSqLiteDatabase.close();
		super.onDestroy();
	}

	@Override
	public void onStart(Intent intent, int startId)
	{
		Log.i(TAG, "onStart");

		try
		{
			int port = intent.getIntExtra("translator_port", 8888);
			if (setTranslatorPort(port))
			{
				EavooClientSocket.closeAll(mClientSockets);
			}
		}
		catch (IOException e)
		{
			e.printStackTrace();
			intent = new Intent(ACTION_SERVICE_START_FAILED);
			sendBroadcast(intent);

			if (mServerSocket != null)
			{
				try
				{
					mServerSocket.close();
				}
				catch (IOException e1)
				{
					e1.printStackTrace();
				}
			}

			return;
		}

		if (mServiceShouldStop)
		{
			mServiceShouldStop = false;

			mLitenThread = new SocketLitenThread();
			mLitenThread.start();
		}

		super.onStart(intent, startId);
	}

	private int sendShortMessage(EavooShortMessage message)
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

	private boolean receiveToDatabase()
	{
		receiveToDatabase(SMS_CONTENT_URI, "_id");
		receiveToDatabase(ICC_CONTENT_URI, "index_on_icc");
		sendDatabaseToClients();

		return true;
	}

	private boolean receiveToDatabase(Uri uriQuery, String idName)
	{
		Log.i(TAG, "receiveToDatabase: uriQuery = " + uriQuery + ", idName = " + idName);

		ContentResolver contentResolver = getContentResolver();
		Cursor cursor = contentResolver.query(uriQuery, null, null, null, null);

		if (cursor == null || cursor.moveToFirst() == false)
		{
			Log.i(TAG, "Cursor is empty!");
			return true;
		}

		int indexId = cursor.getColumnIndex(idName);
		int indexDate = cursor.getColumnIndex("date");
		int indexAddress = cursor.getColumnIndex("address");
		int indexBody = cursor.getColumnIndex("body");

		do
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

			if (message == null || DatabaseInsert(message) > 0)
			{
				Uri uriDelete = uriQuery.buildUpon().appendPath(cursor.getString(indexId)).build();
				Log.i(TAG, "uriDelete = " + uriDelete);
				int delCount = contentResolver.delete(uriDelete, null, null);
				Log.i(TAG, "delCount = " + delCount);
			}
		}
		while (cursor.moveToNext());

		return true;
	}

	class SocketLitenThread extends Thread
	{
		private static final String TAG = "SocketLitenThread";

		synchronized private boolean HandleRequest(Socket socket)
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

		@Override
		public void run()
		{
			Intent intent;

			receiveToDatabase();
			intent = new Intent(ACTION_SERVICE_RUNNING);
			sendBroadcast(intent);

			IntentFilter filter = new IntentFilter();
			filter.addAction(SMS_RECEIVED_ACTION);
			filter.addAction(DATA_SMS_RECEIVED_ACTION);
			registerReceiver(mBroadcastReceiver, filter);

			while (mServerSocket != null)
			{
				Log.i(TAG, "Service ready");

				Socket socket;

				try
				{
					socket = mServerSocket.accept();
				}
				catch (IOException e)
				{
					if (mServiceShouldStop)
					{
						break;
					}

					try
					{
						sleep(1000);
					}
					catch (InterruptedException e1)
					{
						e1.printStackTrace();
					}

					continue;
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

				receiveToDatabase();
			}

			unregisterReceiver(mBroadcastReceiver);
			EavooClientSocket.closeAll(mClientSockets);

			intent = new Intent(ACTION_SERVICE_STOPPED);
			sendBroadcast(intent);

			Log.i(TAG, "Service exit!");
		}
	}
}