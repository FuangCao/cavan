package com.cavan.printer;

import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

import javax.obex.ClientOperation;
import javax.obex.ClientSession;
import javax.obex.HeaderSet;
import javax.obex.ObexTransport;
import javax.obex.ResponseCodes;

import android.app.Activity;
import android.app.AlertDialog.Builder;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.os.Process;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.LinearLayout;
import android.widget.RadioGroup;
import android.widget.Toast;


public class BluetoothPrinterActivity extends Activity
{
	/** Called when the activity is first created. */
	private static final String TAG = "Cavan";
	private static final UUID mUuidSerialPort = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
	private static final UUID mUuidOBEXObjectPush = UUID.fromString("00001123-0000-1000-8000-00805F9B34FB");

	private static final byte[] UUID_DPS =
	{
		0x00, 0x00, 0x11, 0x18, 0x00, 0x00, 0x10, 0x00,
		(byte) 0x80, 0x00, 0x00, (byte) 0x80, 0x5F, (byte) 0x9B, 0x34, (byte) 0xFB
	};
	
	private static final byte[] UUID_PBR =
	{
		0x00, 0x00, 0x11, 0x19, 0x00, 0x00, 0x10, 0x00,
		(byte) 0x80, 0x00, 0x00, (byte) 0x80, 0x5F, (byte) 0x9B, 0x34, (byte) 0xFB
	};

	private static final byte[] UUID_REF_OBJ =
	{
		0x00, 0x00, 0x11, 0x20, 0x00, 0x00, 0x10, 0x00,
		(byte) 0x80, 0x00, 0x00, (byte) 0x80, 0x5F, (byte) 0x9B, 0x34, (byte) 0xFB
	};

	private static final byte[] UUID_URI_REF_OBJ =
	{
		0x00, 0x00, 0x11, 0x21, 0x00, 0x00, 0x10, 0x00,
		(byte) 0x80, 0x00, 0x00, (byte) 0x80, 0x5F, (byte) 0x9B, 0x34, (byte) 0xFB
	};

	private static final byte[] UUID_STS =
	{
		0x00, 0x00, 0x11, 0x23, 0x00, 0x00, 0x10, 0x00,
		(byte) 0x80, 0x00, 0x00, (byte) 0x80, 0x5F, (byte) 0x9B, 0x34, (byte) 0xFB
	};

	private BluetoothAdapter mBluetoothAdapter;
	private Button mButtonStart;
	private Button mButtonStop;
	private Button mButtonRefresh;
	private LinearLayout mLayoutDevices;
	private BluetoothDevice mBluetoothDevice;
	private Button mButtonPrinter;
	private TextView mTextViewStatus;
	
	private String ByteArrayToHexString(byte[] bs)
	{
		if (bs == null)
		{
			return "";
		}

		StringBuilder stringBuilder = new StringBuilder();
		
		for (byte b : bs)
		{
			if ((b & 0xF0) == 0)
			{
				stringBuilder.append("0");
			}

			stringBuilder.append(Integer.toHexString((b >> 4) & 0x0F) + Integer.toHexString(b & 0x0F));
		}

		return stringBuilder.toString();
	}

	private void CavanLog(String message)
	{
		Log.v("Cavan", "\033[1m" + message + "\033[0m");
	}
	
	private void CavanLog(byte[] bs)
	{
		CavanLog(ByteArrayToHexString(bs));
	}

	private void CavanMessage(String message)
	{
		mTextViewStatus.append(ByteArrayToHexString("12345678".getBytes()) + "\n");
		mTextViewStatus.append(message + "\n");
		Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
	}

	class BppObexTransport implements ObexTransport
	{
		private UUID mServerUuid;
	    private BluetoothSocket mBluetoothSocket;
	    private boolean mConnected = false;
	
	    public BppObexTransport(UUID uuid)
	    {
	    	CavanLog("Create BppObexTransport");
	        this.mServerUuid = uuid;
	    }
	
	    public BppObexTransport()
	    {
	    	CavanLog("Create BppObexTransport");
	    	this.mServerUuid = mUuidOBEXObjectPush;
	    }
	
	    public void close() throws IOException
	    {
	    	CavanLog("close");
	    	mBluetoothSocket.close();
	    }
	
	    public DataInputStream openDataInputStream() throws IOException
	    {
	    	CavanLog("openDataInputStream");
	        return new DataInputStream(openInputStream());
	    }
	
	    public DataOutputStream openDataOutputStream() throws IOException
	    {
	    	CavanLog("openDataOutputStream");
	        return new DataOutputStream(openOutputStream());
	    }
	
	    public InputStream openInputStream() throws IOException
	    {
	    	CavanLog("openInputStream");
	        return mBluetoothSocket.getInputStream();
	    }
	
	    public OutputStream openOutputStream() throws IOException
	    {
	    	CavanLog("openOutputStream");
	        return mBluetoothSocket.getOutputStream();
	    }
	
	    public void connect() throws IOException
	    {
	        // TODO Auto-generated method stub
	    	CavanLog("connect");
	    	mBluetoothSocket = mBluetoothDevice.createInsecureRfcommSocketToServiceRecord(mServerUuid);
	    	mBluetoothSocket.connect();
	    	mConnected = true;
	    }
	
	    public void create() throws IOException
	    {
	        // TODO Auto-generated method stub
	    	CavanLog("create");
	    }
	
	    public void disconnect() throws IOException
	    {
	        // TODO Auto-generated method stub
	    	CavanLog("disconnect");
	    }
	
	    public void listen() throws IOException
	    {
	        // TODO Auto-generated method stub
	    	CavanLog("listen");
	    }
	
	    public boolean isConnected() throws IOException
	    {
	    	CavanLog("isConnected");
	        return mConnected;
	    }
	}

	class PrintThread extends Thread
	{
	    private static final String TAG = "Bpp ObexClient";
		private WakeLock mWakeLock;
		private Context mContext;
		private BppObexTransport mTransport;
		private ClientSession mObexClientSession;
	
		public PrintThread(Context context, BppObexTransport transport)
		{
			CavanLog("Create PrintThread");
			PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
			this.mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
	
			this.mContext = context;
			this.mTransport = transport;
		}
	
		private boolean connect()
		{
			CavanLog("Create ClientSession with transport " + mTransport.toString());
	
			try
			{
				CavanLog("Connect to printer");
				mTransport.connect();
				CavanLog("Create OBEX client session");
				mObexClientSession = new ClientSession(mTransport);
			}
			catch (IOException e1)
			{
				CavanLog("OBEX session create error");
				return false;
			}
	
			HeaderSet hsRequest = new HeaderSet();

			hsRequest.setHeader(HeaderSet.TARGET, UUID_DPS);
	
			try
			{
				Log.d(TAG, "Connect to OBEX session");
				HeaderSet hsResponse = mObexClientSession.connect(hsRequest);
				CavanLog("ResponseCode = " + hsResponse.getResponseCode());

				byte[] headerWho = (byte[]) hsResponse.getHeader(HeaderSet.WHO);
				if (headerWho != null)
				{
					CavanLog("HeaderWho:");
					CavanLog(headerWho);
				}
				
				if (hsResponse.mConnectionID == null)
				{
					CavanLog("mConnectionID == null");
				}
				else
				{
					CavanLog(hsResponse.mConnectionID);
				}
				
				if (hsResponse.getResponseCode() == ResponseCodes.OBEX_HTTP_OK)
				{
					return true;
				}
			} catch (IOException e) {
				CavanLog("OBEX session connect error");
			}
	
			try {
				mTransport.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
	
			return false;
		}
		
		private void disconnect()
		{
			CavanLog("disconnect");

			if (mObexClientSession != null)
			{
				try {
					mObexClientSession.disconnect(null);
					mObexClientSession.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
			
			if (mTransport != null)
			{
				try {
					mTransport.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
		
		private boolean SimplePushPrint(String pathname)
		{
			File file = new File(pathname);
			long fileLength = file.length();
			
			if (fileLength == 0)
			{
				CavanLog("File " + pathname + " don't exist");
				return false;
			}

			HeaderSet reqHeaderSet = new HeaderSet();
			reqHeaderSet.setHeader(HeaderSet.NAME, "test.jpg");
			reqHeaderSet.setHeader(HeaderSet.TYPE, "image/jpeg");
			reqHeaderSet.setHeader(HeaderSet.LENGTH, fileLength);
			
			ClientOperation clientOperation;
			try {
				clientOperation = (ClientOperation) mObexClientSession.put(reqHeaderSet);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				clientOperation = null;
			}
			
			if (clientOperation == null)
			{
				CavanLog("clientOperation == null");
				return false;
			}
			
			OutputStream obexOutputStream;
			try {
				obexOutputStream = clientOperation.openOutputStream();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				obexOutputStream = null;
			}
			
			if (obexOutputStream == null)
			{
				CavanLog("obexOutputStream == null");
				try {
					clientOperation.abort();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				return false;
			}

			InputStream obexInputStream;
			try {
				obexInputStream = clientOperation.openInputStream();
			} catch (IOException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
				obexInputStream = null;
			}
			
			if (obexInputStream == null)
			{
				CavanLog("obexInputStream == null");
				
				try {
					obexOutputStream.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}

				try {
					clientOperation.abort();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				
				return false;
			}
			
			FileInputStream fileInputStream;
			try {
				fileInputStream = new FileInputStream(file);
			} catch (FileNotFoundException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				fileInputStream = null;
			}
			
			if (fileInputStream == null)
			{
				CavanLog("fileInputStream == null");

				try {
					obexInputStream.close();
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}

				try {
					obexOutputStream.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}

				try {
					clientOperation.abort();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				return false;
			}
			
			BufferedInputStream bufferedInputStream = new BufferedInputStream(fileInputStream);
			int obexMaxPackageSize = clientOperation.getMaxPacketSize();
			byte[] buff = new byte[obexMaxPackageSize];
			
			boolean boolResult = true;
			
			CavanLog("Start send file");
			
			while (fileLength != 0)
			{
				int sendLength;

				try {
					sendLength = bufferedInputStream.read(buff);
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
					sendLength = -1;
				}
				
				CavanLog("readLength = " + sendLength);
				
				if (sendLength < 0)
				{
					boolResult = false;
					break;
				}
				
				try {
					obexOutputStream.write(buff, 0, sendLength);
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
					boolResult = false;
					break;
				}
				
				int responseCode;
				try {
					responseCode = clientOperation.getResponseCode();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
					boolResult = false;
					break;
				}
				
				CavanLog("responseCode = " + responseCode);

				if (responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK)
				{
					CavanLog("responseCode != ResponseCodes.OBEX_HTTP_CONTINUE && responseCode != ResponseCodes.OBEX_HTTP_OK");
					boolResult = false;
					break;
				}
				
				fileLength -= sendLength;
				
				CavanLog("sendLength = " + sendLength + ", fileLength = " + fileLength);
			}
			
			try {
				bufferedInputStream.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			try {
				obexInputStream.close();
			} catch (IOException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}

			try {
				fileInputStream.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			try {
				obexOutputStream.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			try {
				if (boolResult)
				{
					clientOperation.close();
				}
				else
				{
					clientOperation.abort();
				}
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			return boolResult;
		}
	
		@Override
		public void run()
		{
			Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);
			
			CavanLog("Printer thread running");
	
			mWakeLock.acquire();
			
			if (connect())
			{
				CavanLog("Connect successfully");
			}
			else
			{
				CavanLog("Connect failed");
				return;
			}
			
			if (SimplePushPrint("/mnt/sdcard/test.jpg"))
			{
				CavanLog("Print complete");
			}
			else
			{
				CavanLog("Print failed");
			}
			
			disconnect();
	
			mWakeLock.release();
		}
	}

	class CavanRadioButton extends RadioButton
	{
		private BluetoothDevice mBluetoothDevice;

		public CavanRadioButton(Context context, BluetoothDevice device)
		{
			super(context);
			// TODO Auto-generated constructor stub
			mBluetoothDevice = device;
			setText(mBluetoothDevice.getName());
		}

		public BluetoothDevice getblBluetoothDevice()
		{
			return mBluetoothDevice;
		}
	}

	private boolean ListBluetoothDevices()
	{
		mBluetoothDevice = null;
		mLayoutDevices.removeAllViews();

		if (mBluetoothAdapter == null)
		{
			CavanMessage("getDefaultAdapter failed");
			return false;
		}

		if (mBluetoothAdapter.isEnabled() == false)
		{
			CavanMessage("BluetoothAdapter is disabled");
			return false;
		}

		RadioGroup radioGroup = new RadioGroup(this);

		for (BluetoothDevice bluetoothDevice : mBluetoothAdapter.getBondedDevices())
		{
			CavanRadioButton radioButton = new CavanRadioButton(this, bluetoothDevice);

			radioButton.setOnCheckedChangeListener(new OnCheckedChangeListener()
			{

				@Override
				public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
				{
					// TODO Auto-generated method stub
					if (buttonView instanceof CavanRadioButton && isChecked)
					{
						CavanRadioButton radioButton = (CavanRadioButton) buttonView;

						mBluetoothDevice = radioButton.getblBluetoothDevice();
					}
				}
			});

			radioGroup.addView(radioButton);
		}

		mLayoutDevices.addView(radioGroup);

		return true;
	}

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

		mLayoutDevices = (LinearLayout) findViewById(R.id.linearLayout1);
		mButtonStart = (Button) findViewById(R.id.button1);
		mButtonStop = (Button) findViewById(R.id.button2);
		mButtonRefresh = (Button) findViewById(R.id.button3);
		mButtonPrinter = (Button) findViewById(R.id.button4);
		mTextViewStatus = (TextView) findViewById(R.id.textView1);

		mButtonStart.setOnClickListener(new OnClickListener()
		{

			@Override
			public void onClick(View v)
			{
				// TODO Auto-generated method stub
				if (mBluetoothDevice == null)
				{
					CavanMessage("Please select a bluetooth device");
					return;
				}

				CavanMessage("Start service successfully");
			}
		});

		mButtonStop.setOnClickListener(new OnClickListener()
		{

			@Override
			public void onClick(View v)
			{
				// TODO Auto-generated method stub
				CavanMessage("Stop service");
			}
		});

		mButtonRefresh.setOnClickListener(new OnClickListener()
		{

			@Override
			public void onClick(View v)
			{
				// TODO Auto-generated method stub
				mTextViewStatus.setText("");
				ListBluetoothDevices();
			}
		});

		mButtonPrinter.setOnClickListener(new OnClickListener()
		{
			@Override
			public void onClick(View v)
			{
				// TODO Auto-generated method stub
				if (mBluetoothDevice == null)
				{
					CavanMessage("Please select a bluetooth device");
					return;
				}
				
				CavanMessage("Start Printing");

				BppObexTransport bppObexTransport = new BppObexTransport();
				PrintThread printThread = new PrintThread(BluetoothPrinterActivity.this, bppObexTransport);
				printThread.start();
			}
		});
	}
}
