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
import android.net.Uri;
import android.os.IBinder;
import android.util.Log;

public class AdbSmsTranslatorService extends Service
{
	private static final int MAX_SERVICE_COUNT = 10;
	private static final String TAG = "AdbSmsTranslatorService";
	private static final boolean DEBUG = true;

	public static final String ACTION_SERVICE_RUNNING = "cavan.intent.action.SERVICE_RUNNING";
	public static final String ACTION_SERVICE_STOPPED = "cavan.intent.action.SERVICE_STOPPED";
	public static final String ACTION_SERVICE_START_FAILED = "cavan.intent.action.SERVICE_START_FAILED";
	public static final String ACTION_SERVICE_STOP_FAILED = "cavan.intent.action.SERVICE_STOP_FAILED";
	public static final String ACTION_SMS_RECEIVED = "android.provider.Telephony.SMS_RECEIVED";

	private ServerSocket mServerSocket;
	private EavooClientSocket[] mClientSockets = new EavooClientSocket[MAX_SERVICE_COUNT];
	private boolean mServiceShouldStop = true;
	private SocketLitenThread mLitenThread;
	private SQLiteOpenHelperMessage mSqLiteOpenHelperMessage;

	private boolean deleteAllSms()
	{
		Uri uri = Uri.parse("content://sms/sent");
		ContentResolver resolver = getContentResolver();
		Cursor cursor = resolver.query(uri, new String[]{"_id", "thread_id"}, null, null, null);
		if (cursor == null || cursor.moveToFirst() == false)
		{
			return false;
		}

		do
		{
			long threadId = cursor.getLong(1);
			resolver.delete(Uri.parse("content://sms/conversations/" + threadId), null, null);
			Log.d(TAG, "threadId =  " + threadId);
		} while (cursor.moveToNext());

		return true;
	}

	private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver()
	{
		@Override
		public void onReceive(Context context, Intent intent)
		{
			Log.i(TAG, "action = " + intent.getAction());

			Object []pdus = (Object []) intent.getExtras().get("pdus");
			if (pdus == null)
			{
				Log.e(TAG, "object == null");
				return;
			}

			try
			{
				EavooShortMessage message = new EavooShortMessage(pdus);
				if (sendShortMessageToClients(message) <= 0)
				{
					mSqLiteOpenHelperMessage.insert(message);
				}
			}
			catch (Exception e)
			{
				e.printStackTrace();
			}

			deleteAllSms();
		}
	};

	private int sendShortMessageToClients(EavooShortMessage message)
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

	protected void insertIntoDatabase(EavooShortMessage message)
	{
		if (DEBUG)
		{
			Log.i(TAG, "insertIntoDatabase: " + message);
		}

		mSqLiteOpenHelperMessage.insert(message);
	}

	private void closeClientSockets()
	{
		for (int i = mClientSockets.length - 1; i >= 0; i--)
		{
			if (mClientSockets[i] == null)
			{
				continue;
			}

			mClientSockets[i].close();
			mClientSockets[i] = null;
		}
	}

	private int findFreeClientSocket()
	{
		for (int i = mClientSockets.length - 1; i >= 0; i--)
		{
			if (mClientSockets[i] == null)
			{
				return i;
			}

			if (mClientSockets[i].test_network() < 0)
			{
				mClientSockets[i].close();
				return i;
			}
		}

		return -1;
	}

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
		mSqLiteOpenHelperMessage = new SQLiteOpenHelperMessage(this, null);

		super.onCreate();
	}

	@Override
	public void onDestroy()
	{
		Log.i(TAG, "onDestroy");

		unregisterReceiver(mBroadcastReceiver);

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

		closeClientSockets();

		super.onDestroy();

		Intent intent = new Intent(ACTION_SERVICE_STOPPED);
		sendBroadcast(intent);
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
				closeClientSockets();
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

		IntentFilter filter = new IntentFilter(ACTION_SMS_RECEIVED);
		registerReceiver(mBroadcastReceiver, filter);

		deleteAllSms();

		intent = new Intent(ACTION_SERVICE_RUNNING);
		sendBroadcast(intent);
	}

	class SocketLitenThread extends Thread
	{
		private static final String TAG = "SocketLitenThread";

		synchronized private boolean HandleRequest(Socket socket)
		{
			int index = findFreeClientSocket();
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

			if (sendDatabaseToClient(mClientSockets[index]) == false)
			{
				mClientSockets[index].close();
				mClientSockets[index] = null;
				return false;
			}

			return true;
		}

		private boolean sendDatabaseToClient(EavooClientSocket socket)
		{
			Cursor cursor = mSqLiteOpenHelperMessage.quertAll();
			if (cursor.moveToFirst() == false)
			{
				return true;
			}

			do
			{
				EavooShortMessage message = new EavooShortMessage(cursor);
				if (DEBUG)
				{
					Log.i(TAG, "sendDatabaseToClient: " + message);
				}

				int ret = socket.write(message);
				if (ret < 0)
				{
					return false;
				}

				mSqLiteOpenHelperMessage.delete(cursor.getInt(0));
			}
			while (cursor.moveToNext());

			return true;
		}

		@Override
		public void run()
		{
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
			}

			Log.i(TAG, "Service exit!");
		}
	}
}