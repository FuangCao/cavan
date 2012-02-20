package com.eavoo.printer;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.eavoo.printer.BluetoothPrintService.BluetoothPrintBinder;

public class BluetoothPrinterActivity extends Activity
{
	private BluetoothPrintService mBluetoothPrintService;
	private BluetoothAdapter mBluetoothAdapter;
	private Button mButtonJobBasePrint;
	private Button mButtonRefresh;
	private LinearLayout mLayoutDevices;
	private BluetoothDevice mBluetoothDevice;
	private Button mButtonSimplePushPrint;
	private TextView mTextViewStatus;
	private EditText mEditTextFilePath;
	private BluetoothPrintJob mPrintJob = new BluetoothPrintJob();

	private ServiceConnection mServiceConnection = new ServiceConnection()
	{
		@Override
		public void onServiceDisconnected(ComponentName name)
		{
			Log.v("Cavan", "onServiceDisconnected");
			mBluetoothPrintService = null;
		}

		@Override
		public void onServiceConnected(ComponentName name, IBinder service)
		{
			Log.v("Cavan", "onServiceConnected");
			BluetoothPrintBinder binder = (BluetoothPrintBinder) service;
			mBluetoothPrintService = binder.getService();
		}
	};

	private OnClickListener mOnClickListener = new OnClickListener()
	{
		@Override
		public void onClick(View v)
		{
			switch (v.getId())
			{
			case R.id.main_button1:
				if (mBluetoothDevice == null)
				{
					CavanMessage("Please select a bluetooth device");
					return;
				}

				mPrintJob.setFileName(getFileName());
				mBluetoothPrintService.JobBasePrint(mHandler, mBluetoothDevice, mPrintJob);
				break;

			case R.id.main_button2:
				if (mBluetoothDevice == null)
				{
					CavanMessage("Please select a bluetooth device");
					return;
				}

				mPrintJob.setFileName(getFileName());
				mBluetoothPrintService.SimplePushPrint(mHandler, mBluetoothDevice, mPrintJob);
				break;

			case R.id.main_button3:
				mTextViewStatus.setText("");
				ListBluetoothDevices();
				if (mBluetoothDevice != null)
				{
					mBluetoothPrintService.GetPrinterAttribute(mHandler, mBluetoothDevice);
				}
				break;

			case R.id.main_editText1:
				Intent intent = new Intent(Intent.ACTION_VIEW);
				intent.setClass(getApplicationContext(), FileBrowserActivity.class);
				startActivityForResult(intent, 0);
				break;

			default:
				CavanMessage("unknown onclick event");
			}
		}
	};

	private Handler mHandler = new Handler()
	{
		@Override
		public void handleMessage(Message msg)
		{
			switch (msg.what)
			{
			case BluetoothBasePrinter.BPP_MSG_GET_PRINTER_ATTRIBUTE_COMPLETE:
				if (msg.arg1 < 0)
				{
					CavanMessage("get printer attribute failed");
				}
				else
				{
					CavanMessage("get printer attribute complete");
					Bundle data = msg.getData();
					BluetoothPrinterAttribute attribute = (BluetoothPrinterAttribute) data.getSerializable("PrinterAttribute");
					if (attribute != null)
					{
						ShowPrinterAttribute(attribute);
					}
				}
				break;

			case BluetoothBasePrinter.BPP_MSG_JOB_BASE_PRINT_COMPLETE:
				if (msg.arg1 < 0)
				{
					CavanMessage("job base print failed");
				}
				else
				{
					CavanMessage("job base print complete");
				}
				break;

			case BluetoothBasePrinter.BPP_MSG_SIMPLE_PUSH_PRINT_COMPLETE:
				if (msg.arg1 < 0)
				{
					CavanMessage("simple push print failed");
				}
				else
				{
					CavanMessage("simple push print complete");
				}
				break;

			default:
				CavanMessage("unknown message = " + msg.what);
			}
		}
	};

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
			mBluetoothDevice = device;
			setText(mBluetoothDevice.getName());
		}

		public BluetoothDevice getblBluetoothDevice()
		{
			return mBluetoothDevice;
		}
	}

	private void ShowPrinterAttribute(BluetoothPrinterAttribute attribute)
	{
		mTextViewStatus.append("PrinterName = " + attribute.getPrinterName() + "\n");
		mTextViewStatus.append("BasicTextPageHeight = " + attribute.getBasicTextPageHeight() + "\n");
		mTextViewStatus.append("BasicTextPageWidth = " + attribute.getBasicTextPageWidth() + "\n");
		mTextViewStatus.append("PrinterState = " + attribute.getPrinterState() + "\n");
		mTextViewStatus.append("PrinterStateReasons = " + attribute.getPrinterStateReasons() + "\n");
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
		CavanRadioButton radioButton = null;

		for (BluetoothDevice bluetoothDevice : mBluetoothAdapter.getBondedDevices())
		{
			radioButton = new CavanRadioButton(this, bluetoothDevice);

			radioButton.setOnCheckedChangeListener(new OnCheckedChangeListener()
			{
				@Override
				public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
				{
					if (buttonView instanceof CavanRadioButton && isChecked)
					{
						CavanRadioButton radioButton = (CavanRadioButton) buttonView;

						mBluetoothDevice = radioButton.getblBluetoothDevice();
					}
				}
			});

			radioGroup.addView(radioButton);
		}

		if (radioButton != null)
		{
			radioButton.setChecked(true);
		}

		mLayoutDevices.addView(radioGroup);

		return true;
	}

	public String getFileName()
	{
		return mEditTextFilePath.getText().toString();
	}

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

		mLayoutDevices = (LinearLayout) findViewById(R.id.main_linearLayout1);
		mButtonJobBasePrint = (Button) findViewById(R.id.main_button1);
		mButtonSimplePushPrint = (Button) findViewById(R.id.main_button2);
		mButtonRefresh = (Button) findViewById(R.id.main_button3);
		mTextViewStatus = (TextView) findViewById(R.id.main_textView1);
		mEditTextFilePath = (EditText) findViewById(R.id.main_editText1);

		mButtonJobBasePrint.setOnClickListener(mOnClickListener);
		mButtonRefresh.setOnClickListener(mOnClickListener);
		mButtonSimplePushPrint.setOnClickListener(mOnClickListener);
		mEditTextFilePath.setOnClickListener(mOnClickListener);

		ListBluetoothDevices();

		Intent service = new Intent(this, BluetoothPrintService.class);
		bindService(service, mServiceConnection, Context.BIND_AUTO_CREATE);
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		switch (requestCode)
		{
		case 0:
			if (data != null)
			{
				mEditTextFilePath.setText(data.getStringExtra("pathname"));
			}
			break;

		default:
			Log.v("Cavan", "unknown requestCode = " + requestCode);
		}
	}
}
