package com.eavoo.printer;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.LinearLayout;
import android.widget.RadioGroup;
import android.widget.Toast;


public class BluetoothPrinterActivity extends Activity
{
	/** Called when the activity is first created. */
	private String mFileName = "/mnt/sdcard/printer.txt";

	private BluetoothAdapter mBluetoothAdapter;
	private Button mButtonJobBasePrint;
	private Button mButtonRefresh;
	private LinearLayout mLayoutDevices;
	private BluetoothDevice mBluetoothDevice;
	private Button mButtonSimplePushPrint;
	private TextView mTextViewStatus;
	private EditText mEditTextFilePath;

	private void CavanMessage(String message)
	{
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

	public boolean updateFileName()
	{
		mFileName = mEditTextFilePath.getText().toString();
		if (mFileName == "")
		{
			CavanMessage("Please input file path");
			return false;
		}

		return true;
	}

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

		mLayoutDevices = (LinearLayout) findViewById(R.id.linearLayout1);
		mButtonJobBasePrint = (Button) findViewById(R.id.button1);
		mButtonSimplePushPrint = (Button) findViewById(R.id.button2);
		mButtonRefresh = (Button) findViewById(R.id.button3);
		mTextViewStatus = (TextView) findViewById(R.id.textView1);
		mEditTextFilePath = (EditText) findViewById(R.id.editText1);

		mButtonJobBasePrint.setOnClickListener(new OnClickListener()
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
				if (updateFileName())
				{
					JobBasePrinter jobBasePrinter = new JobBasePrinter(BluetoothPrinterActivity.this, bppObexTransport, mFileName, null);
					jobBasePrinter.start();
				}
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

		mButtonSimplePushPrint.setOnClickListener(new OnClickListener()
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
				if (updateFileName())
				{
					SimplePushPrinter printer = new SimplePushPrinter(BluetoothPrinterActivity.this, bppObexTransport, mFileName, null);
					printer.start();
				}
			}
		});

		mEditTextFilePath.setText(mFileName);
	}
}
