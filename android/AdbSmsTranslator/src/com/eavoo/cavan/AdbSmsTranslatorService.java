package com.eavoo.cavan;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.Uri;
import android.os.IBinder;
import android.telephony.SmsMessage;
import android.util.Log;

public class AdbSmsTranslatorService extends Service
{
	private static final int MAX_SERVICE_COUNT = 10;
	private static final String TAG = "AdbSmsTranslatorService";

	public static final String ACTION_SERVICE_RUNNING = "cavan.intent.action.SERVICE_RUNNING";
	public static final String ACTION_SERVICE_STOPPED = "cavan.intent.action.SERVICE_STOPPED";
	public static final String ACTION_SERVICE_START_FAILED = "cavan.intent.action.SERVICE_START_FAILED";
	public static final String ACTION_SERVICE_STOP_FAILED = "cavan.intent.action.SERVICE_STOP_FAILED";
	public static final String ACTION_SMS_RECEIVED = "android.provider.Telephony.SMS_RECEIVED";

	private ServerSocket mServerSocket;
	private OutputStream[] mOutputStreams = new OutputStream[MAX_SERVICE_COUNT];
	private Socket[] mClientSockets = new Socket[MAX_SERVICE_COUNT];
	private boolean mServiceShouldStop = true;
	private SocketLitenThread mLitenThread;
	private File mFile;
	private FileOutputStream mFileOutputStream;

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

			Object object = intent.getExtras().get("pdus");
			if (object == null)
			{
				Log.e(TAG, "object == null");
				return;
			}

			StringBuilder builder = new StringBuilder();
			int count = 0;

			for (Object pdus : (Object[]) object)
			{
				byte[] pdusmessage = (byte[]) pdus;

				SmsMessage sms = SmsMessage.createFromPdu(pdusmessage);

				if (count == 0)
				{
					String mobile = sms.getOriginatingAddress();// 发送短信的手机号码
					builder.append(mobile + ",");

					Date date = new Date(sms.getTimestampMillis());
					SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
					String time = format.format(date); // 得到发送时间
					builder.append(time + ',');
				}

				String content = sms.getMessageBody(); // 短信内容
				builder.append(content);

				count++;
			}

			builder.append("\r\n");
			sendByteToClients(builder.toString().getBytes());
			Log.i(TAG, "SmsMessage count = " + count);

			deleteAllSms();
		}
	};

	private void sendByteToClients(byte[] buff)
	{
		for (int i = mOutputStreams.length - 1; i >= 0; i--)
		{
			if (mOutputStreams[i] == null)
			{
				continue;
			}

			try
			{
				mOutputStreams[i].write(buff);
				continue;
			}
			catch (IOException e)
			{
				e.printStackTrace();
			}

			try
			{
				mOutputStreams[i].close();
				mClientSockets[i].close();
			}
			catch (IOException e)
			{
				e.printStackTrace();
			}

			mOutputStreams[i] = null;
			mClientSockets[i] = null;
		}

		if (mFileOutputStream != null)
		{
			try
			{
				mFileOutputStream.write(buff);
				mFileOutputStream.flush();
			}
			catch (IOException e)
			{
				try
				{
					mFileOutputStream.close();
					mFileOutputStream = null;
				}
				catch (IOException e1)
				{
					e1.printStackTrace();
				}

				e.printStackTrace();
			}
		}
	}

	private void closeOutPutStreams() throws IOException
	{
		for (int i = mOutputStreams.length - 1; i >= 0; i--)
		{
			if (mOutputStreams[i] == null)
			{
				continue;
			}

			mOutputStreams[i].close();
			mOutputStreams[i] = null;
		}
	}

	private void closeClientSockets() throws IOException
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

		try
		{
			closeOutPutStreams();
			closeClientSockets();
		}
		catch (IOException e)
		{
			Intent intent = new Intent(ACTION_SERVICE_STOP_FAILED);
			sendBroadcast(intent);
			e.printStackTrace();
			return;
		}

		if (mFileOutputStream != null)
		{
			try
			{
				mFileOutputStream.close();
				mFileOutputStream = null;
			}
			catch (IOException e)
			{
				Intent intent = new Intent(ACTION_SERVICE_STOP_FAILED);
				sendBroadcast(intent);
				e.printStackTrace();
				return;
			}
		}

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
				closeOutPutStreams();
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

		if (mFile == null)
		{
			String pathname = intent.getStringExtra("pathname");
			if (pathname != null)
			{
				mFile = new File(pathname);
			}
		}

		if (mFile != null && mFileOutputStream == null)
		{
			try
			{
				mFileOutputStream = new FileOutputStream(mFile, true);
			}
			catch (FileNotFoundException e)
			{
				mFileOutputStream = null;
				e.printStackTrace();
			}
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

		private boolean HandleRequest(Socket socket)
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
				mOutputStreams[index] = socket.getOutputStream();
			}
			catch (IOException e)
			{
				mOutputStreams[index] = null;
				e.printStackTrace();
				return false;
			}

			FileInputStream fileInputStream = null;

			try
			{
				if (mFile != null)
				{
					fileInputStream = new FileInputStream(mFile);
					byte[] buff = new byte[1024];
					int readLen;

					while (true)
					{
						readLen = fileInputStream.read(buff);
						if (readLen <= 0)
						{
							break;
						}

						mOutputStreams[index].write(buff, 0, readLen);
					}
				}
			}
			catch (FileNotFoundException e)
			{
				e.printStackTrace();
			}
			catch (IOException e)
			{
				e.printStackTrace();
			}

			if (fileInputStream != null)
			{
				try
				{
					fileInputStream.close();
				}
				catch (IOException e)
				{
					e.printStackTrace();
				}
			}

			mClientSockets[index] = socket;

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