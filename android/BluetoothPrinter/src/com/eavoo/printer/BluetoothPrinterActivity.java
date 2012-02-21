package com.eavoo.printer;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.eavoo.printer.BluetoothPrintService.BluetoothPrintBinder;

public class BluetoothPrinterActivity extends Activity
{
	private BluetoothPrintService mBluetoothPrintService;
	private BluetoothAdapter mBluetoothAdapter;
	private Button mButtonJobBasePrint;
	private Button mButtonRefresh;
	private RadioGroup mRadioGroupDevice;
	private BluetoothDevice mBluetoothDevice;
	private Button mButtonSimplePushPrint;
	private TextView mTextViewStatus;
	private EditText mEditTextFilePath;
	private BluetoothPrintJob mPrintJob = new BluetoothPrintJob();
	private Spinner mSpinnerFileType;
	private StringListAdapter mAdapterFileType = new StringListAdapter(this);
	private Spinner mSpinnerMediaType;
	private StringListAdapter mAdapterMediaType = new StringListAdapter(this);
	private Spinner mSpinnerMediaSize;
	private StringListAdapter mAdapterMediaSize = new StringListAdapter(this);
	private Spinner mSpinnerPrintQuality;
	private StringListAdapter mAdapterPrintQuality = new StringListAdapter(this);

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
			ListBluetoothDevices();
		}
	};

	private OnClickListener mOnClickListener = new OnClickListener()
	{
		@Override
		public void onClick(View v)
		{
			switch (v.getId())
			{
			case R.id.main_button_job_print:
				if (mBluetoothDevice == null)
				{
					CavanMessage("Please select a bluetooth device");
					return;
				}

				if (mBluetoothPrintService != null)
				{
					mPrintJob.setFileName(getFileName());
					mBluetoothPrintService.JobBasePrint(mHandler, mBluetoothDevice, mPrintJob);
				}
				else
				{
					CavanMessage("mBluetoothPrintService is null");
				}
				break;

			case R.id.main_button_simple_print:
				if (mBluetoothDevice == null)
				{
					CavanMessage("Please select a bluetooth device");
					return;
				}

				if (mBluetoothPrintService != null)
				{
					mPrintJob.setFileName(getFileName());
					mBluetoothPrintService.SimplePushPrint(mHandler, mBluetoothDevice, mPrintJob);
				}
				else
				{
					CavanMessage("mBluetoothPrintService is null");
				}
				break;

			case R.id.main_button_refresh:
				mTextViewStatus.setText("Refresh bluetooth device list");
				ListBluetoothDevices();
				break;

			case R.id.main_editText_pathname:
				Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(mEditTextFilePath.getText().toString()));
				intent.setClass(getApplicationContext(), FileBrowserActivity.class);
				startActivityForResult(intent, 0);
				break;

			default:
				CavanMessage("unknown onclick event");
			}
		}
	};

	private boolean GetPrinterAttribute()
	{
		if (mBluetoothPrintService == null)
		{
			Message message = Message.obtain(mHandler);
			message.what = BluetoothBasePrinter.BPP_MSG_GET_PRINTER_ATTRIBUTE_REQUEST;
			mHandler.sendMessageDelayed(message, 1000);
			CavanMessage("mBluetoothPrintService is null");
			return false;
		}
		else
		{
			mBluetoothPrintService.GetPrinterAttribute(mHandler, mBluetoothDevice);
			return true;
		}
	}

	private OnCheckedChangeListener mOnCheckedChangeListener = new OnCheckedChangeListener()
	{
		@Override
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
		{
			if (buttonView instanceof CavanRadioButton && isChecked)
			{
				CavanRadioButton radioButton = (CavanRadioButton) buttonView;

				mBluetoothDevice = radioButton.getblBluetoothDevice();
				GetPrinterAttribute();
			}
		}
	};

	private TextWatcher mTextWatcher = new TextWatcher()
	{
		@Override
		public void onTextChanged(CharSequence s, int start, int before, int count)
		{
			String pathname = s.toString();
			mPrintJob.setFileName(pathname);
			String filetype = BluetoothPrintJob.GetFileMimeTypeByName(pathname);
			int index = mAdapterFileType.indexOf(filetype);
			if (index >= 0)
			{
				mSpinnerFileType.setSelection(index);
			}
		}

		@Override
		public void beforeTextChanged(CharSequence s, int start, int count, int after)
		{
		}

		@Override
		public void afterTextChanged(Editable s)
		{
		}
	};

	private OnItemSelectedListener mOnItemSelectedListener = new OnItemSelectedListener()
	{
		@Override
		public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3)
		{
			TextView view = (TextView) arg1;
			String text = view.getText().toString();

			switch (arg0.getId())
			{
			case R.id.main_spinner_file_type:
				mPrintJob.setDocumentFormat(text);
				break;

			case R.id.main_spinner_media_size:
				mPrintJob.setMediaSize(text);
				break;

			case R.id.main_spinner_media_type:
				mPrintJob.setMediaType(text);
				break;

			case R.id.main_spinner_print_quality:
				mPrintJob.setPrintQuality(text);
				break;
			}
		}

		@Override
		public void onNothingSelected(AdapterView<?> arg0)
		{
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
					GetPrinterAttribute();
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
					GetPrinterAttribute();
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
					GetPrinterAttribute();
				}
				else
				{
					CavanMessage("simple push print complete");
				}
				break;

			case BluetoothBasePrinter.BPP_MSG_CONNECT_FAILED:
				CavanMessage("Create connect failed");
				break;

			case BluetoothBasePrinter.BPP_MSG_GET_PRINTER_ATTRIBUTE_REQUEST:
				CavanMessage("Get print attribute request");
				GetPrinterAttribute();
				break;

			default:
				CavanMessage("unknown message = " + msg.what);
			}
		}
	};

	private void CavanMessage(String message)
	{
		Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
		mTextViewStatus.setText(message);
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
		mTextViewStatus.setText(String.format("%s: %s", attribute.getPrinterState(), attribute.getPrinterStateReasons()));

		mAdapterFileType.setStringList(attribute.getDocumentFormatsSupported());
		mSpinnerFileType.setSelection(mAdapterFileType.indexOf(mPrintJob.getDocumentFormat()));

		mAdapterMediaType.setStringList(attribute.getMediaTypesSupported());
		mSpinnerMediaType.setSelection(mAdapterMediaType.indexOf(mPrintJob.getMediaType()));

		mAdapterMediaSize.setStringList(attribute.getMediaSizesSupported());
		mSpinnerMediaSize.setSelection(mAdapterMediaSize.indexOf(mPrintJob.getMediaSize()));

		mAdapterPrintQuality.setStringList(attribute.getPrintQualitySupported());
		mSpinnerPrintQuality.setSelection(mAdapterPrintQuality.indexOf(mPrintJob.getPrintQuality()));
	}

	private boolean ListBluetoothDevices()
	{
		mBluetoothDevice = null;

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

		mRadioGroupDevice.removeAllViews();
		CavanRadioButton radioButton = null;

		for (BluetoothDevice bluetoothDevice : mBluetoothAdapter.getBondedDevices())
		{
			radioButton = new CavanRadioButton(this, bluetoothDevice);

			radioButton.setOnCheckedChangeListener(mOnCheckedChangeListener);

			mRadioGroupDevice.addView(radioButton);
		}

		if (radioButton != null)
		{
			radioButton.setChecked(true);
		}

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

		Intent service = new Intent(this, BluetoothPrintService.class);
		bindService(service, mServiceConnection, Context.BIND_AUTO_CREATE);

		mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

		mButtonJobBasePrint = (Button) findViewById(R.id.main_button_job_print);
		mButtonJobBasePrint.setOnClickListener(mOnClickListener);

		mButtonSimplePushPrint = (Button) findViewById(R.id.main_button_simple_print);
		mButtonSimplePushPrint.setOnClickListener(mOnClickListener);

		mButtonRefresh = (Button) findViewById(R.id.main_button_refresh);
		mButtonRefresh.setOnClickListener(mOnClickListener);

		mTextViewStatus = (TextView) findViewById(R.id.main_textView_status);
		mEditTextFilePath = (EditText) findViewById(R.id.main_editText_pathname);
		mEditTextFilePath.setOnClickListener(mOnClickListener);
		mEditTextFilePath.addTextChangedListener(mTextWatcher);

		mRadioGroupDevice = (RadioGroup) findViewById(R.id.main_radioGroup_device);

		mSpinnerFileType = (Spinner) findViewById(R.id.main_spinner_file_type);
		mSpinnerFileType.setAdapter(mAdapterFileType);
		mSpinnerFileType.setOnItemSelectedListener(mOnItemSelectedListener);

		mSpinnerMediaType = (Spinner) findViewById(R.id.main_spinner_media_type);
		mSpinnerMediaType.setAdapter(mAdapterMediaType);
		mSpinnerMediaType.setOnItemSelectedListener(mOnItemSelectedListener);

		mSpinnerMediaSize = (Spinner) findViewById(R.id.main_spinner_media_size);
		mSpinnerMediaSize.setAdapter(mAdapterMediaSize);
		mSpinnerMediaSize.setOnItemSelectedListener(mOnItemSelectedListener);

		mSpinnerPrintQuality = (Spinner) findViewById(R.id.main_spinner_print_quality);
		mSpinnerPrintQuality.setAdapter(mAdapterPrintQuality);
		mSpinnerPrintQuality.setOnItemSelectedListener(mOnItemSelectedListener);
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

class StringListAdapter extends BaseAdapter
{
	private Context mContext;
	private List<String> mListStrings = new ArrayList<String>();

	public StringListAdapter(Context context)
	{
		this.mContext = context;
	}

	@Override
	public int getCount()
	{
		return mListStrings.size();
	}

	@Override
	public Object getItem(int position)
	{
		return null;
	}

	@Override
	public long getItemId(int position)
	{
		return position;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent)
	{
		TextView view = new TextView(mContext);
		view.setText(mListStrings.get(position));
		view.setSingleLine();
		view.setTextSize(18);
		view.setTextColor(Color.BLACK);

		return view;
	}

	public boolean setStringList(List<String> strings)
	{
		if (strings == null)
		{
			return false;
		}

		mListStrings = strings;
		notifyDataSetChanged();

		return true;
	}

	public int indexOf(String text)
	{
		int i = 0;

		for (String iterable : mListStrings)
		{
			if (iterable.equals(text))
			{
				return i;
			}

			i++;
		}

		return -1;
	}
}