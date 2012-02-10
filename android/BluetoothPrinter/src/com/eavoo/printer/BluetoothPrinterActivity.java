package com.eavoo.printer;

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

				BppObexTransport bppObexTransport = new BppObexTransport(mBluetoothDevice);
				JobBasePrinter jobBasePrinter = new JobBasePrinter(BluetoothPrinterActivity.this, bppObexTransport);
				jobBasePrinter.start();
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

				BppObexTransport bppObexTransport = new BppObexTransport(mBluetoothDevice);
				SimplePushPrinter printer = new SimplePushPrinter(BluetoothPrinterActivity.this, bppObexTransport);
				printer.start();
			}
		});
	}
}
