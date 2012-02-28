package com.eavoo.printer;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.res.AssetManager;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.Gravity;
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
	private static final int DIALOG_PROGRESS_ID = 0;
	private static final int DIALOG_ALERT_YES_ID = 1;

	private static final String TAG = "BluetoothPrinterActivity";

	private String mBinaryFilePdfToJpeg;

	private BluetoothPrintService mBluetoothPrintService;
	private BluetoothAdapter mBluetoothAdapter;

	private Button mButtonJobBasePrint;
	private Button mButtonSimplePushPrint;
	private Button mButtonRefresh;
	private Button mButtonPrintScan;

	private RadioGroup mRadioGroupDevice;
	private BluetoothDevice mBluetoothDevice;

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
	private ProgressDialog mProgressDialog;
	private boolean mProgressDialogRunning = false;
	private AlertDialog mAlertDialogYes;
	private TextView mTextViewlertDialogYes;

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
			mBluetoothPrintService.setBluetoothPrinterActivity(BluetoothPrinterActivity.this);
			listBluetoothDevices();
		}
	};

	@Override
	protected Dialog onCreateDialog(int id, Bundle bundle)
	{
		String message = bundle.getString("message");
		if (message == null)
		{
			Log.e(TAG, "message == null");
			return null;
		}

		switch (id)
		{
		case DIALOG_PROGRESS_ID:
			mProgressDialog = new ProgressDialog(this);
			mProgressDialog.setMessage(message);
			mProgressDialog.setCancelable(false);
			return mProgressDialog;

		case DIALOG_ALERT_YES_ID:
			mTextViewlertDialogYes = new TextView(this);
			mTextViewlertDialogYes.setText(message);
			mTextViewlertDialogYes.setTextSize(18);
			mTextViewlertDialogYes.setHeight(50);
			mTextViewlertDialogYes.setGravity(Gravity.CENTER_HORIZONTAL | Gravity.CENTER_VERTICAL);

			AlertDialog.Builder builder = new AlertDialog.Builder(this);
			builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener()
			{
				@Override
				public void onClick(DialogInterface dialog, int which)
				{
					Log.v(TAG, "DialogInterface.OnClickListener");
				}
			});

			builder.setView(mTextViewlertDialogYes);
			mAlertDialogYes = builder.create();
			mAlertDialogYes.setCancelable(false);

			return mAlertDialogYes;
		}

		return null;
	}

	private Bundle buileBundle(String message)
	{
		Bundle bundle = new Bundle();
		bundle.putString("message", message);

		return bundle;
	}

	private void showProgressDialog(String message)
	{
		if (mProgressDialog != null)
		{
			if (mProgressDialogRunning)
			{
				dismissDialog(DIALOG_PROGRESS_ID);
			}

			mProgressDialog.setMessage(message);
			showDialog(DIALOG_PROGRESS_ID);
		}
		else
		{
			showDialog(DIALOG_PROGRESS_ID, buileBundle(message));
		}

		mProgressDialogRunning = true;
	}

	private void closeProgressDialog()
	{
		if (mProgressDialogRunning)
		{
			dismissDialog(DIALOG_PROGRESS_ID);
			mProgressDialogRunning = false;
		}
	}

	private void showAlertDialogYes(String message)
	{
		if (mAlertDialogYes != null)
		{
			mTextViewlertDialogYes.setText(message);
			showDialog(DIALOG_ALERT_YES_ID);
		}
		else
		{
			showDialog(DIALOG_ALERT_YES_ID, buileBundle(message));
		}
	}

	private boolean getPrinterAttribute()
	{
		if (mBluetoothDevice == null)
		{
			showAlertDialogYes("Please select a bluetooth device");
			return false;
		}

		if (mBluetoothPrintService == null)
		{
			Message message = Message.obtain(mHandler);
			message.what = BluetoothBasePrinter.BPP_MSG_GET_PRINTER_ATTRIBUTE_REQUEST;
			mHandler.sendMessageDelayed(message, 1000);
			CavanMessage("mBluetoothPrintService is null");
			return false;
		}

		showProgressDialog(String.format("Get %s's Attribute ...", mBluetoothDevice.getName()));
		mBluetoothPrintService.GetPrinterAttribute();

		return true;
	}

	private String checkBluetoothPrint()
	{
		if (mBluetoothDevice == null)
		{
			showAlertDialogYes("Please select a bluetooth device");
			return null;
		}

		if (mBluetoothPrintService == null)
		{
			showAlertDialogYes("Bluetooth print service is not ready, please retry");
			return null;
		}

		String filename = mPrintJob.getFileName();
		if (filename == null || filename.isEmpty())
		{
			showAlertDialogYes("Please select a file");
			return null;
		}

		return filename;
	}

	private boolean JobBasePrint()
	{
		String filename = checkBluetoothPrint();
		if (filename == null)
		{
			return false;
		}

		String message = String.format("JobBasePrint %s ...", filename);
		mTextViewStatus.setText(message);
		showProgressDialog(message);

		return mBluetoothPrintService.JobBasePrint();
	}

	private boolean SimplePushPrint()
	{
		String filename = checkBluetoothPrint();
		if (filename == null)
		{
			return false;
		}

		String message = String.format("JobBasePrint %s ...", filename);
		mTextViewStatus.setText(message);
		showProgressDialog(message);

		return mBluetoothPrintService.SimplePushPrint();
	}

	private OnClickListener mOnClickListener = new OnClickListener()
	{
		@Override
		public void onClick(View v)
		{
			switch (v.getId())
			{
			case R.id.main_button_job_print:
				JobBasePrint();
				break;

			case R.id.main_button_simple_print:
				SimplePushPrint();
				break;

			case R.id.main_button_refresh:
				getPrinterAttribute();
				break;

			case R.id.main_button_printer_scan:
				listBluetoothDevices();
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

	private OnCheckedChangeListener mOnCheckedChangeListener = new OnCheckedChangeListener()
	{
		@Override
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
		{
			if (buttonView instanceof CavanRadioButton && isChecked)
			{
				CavanRadioButton radioButton = (CavanRadioButton) buttonView;

				mBluetoothDevice = radioButton.getblBluetoothDevice();
				getPrinterAttribute();
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
			closeProgressDialog();

			switch (msg.what)
			{
			case BluetoothBasePrinter.BPP_MSG_GET_PRINTER_ATTRIBUTE_COMPLETE:
				if (msg.arg1 < 0)
				{
					CavanMessage("get printer attribute failed");
					getPrinterAttribute();
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
					showAlertDialogYes("job base print failed!");
				}
				else
				{
					showAlertDialogYes("job base print complete");
				}
				break;

			case BluetoothBasePrinter.BPP_MSG_SIMPLE_PUSH_PRINT_COMPLETE:
				if (msg.arg1 < 0)
				{
					showAlertDialogYes("simple push print failed!");
				}
				else
				{
					showAlertDialogYes("simple push print complete");
				}
				break;

			case BluetoothBasePrinter.BPP_MSG_CONNECT_FAILED:
				CavanMessage("Create connect failed");
				break;

			case BluetoothBasePrinter.BPP_MSG_GET_PRINTER_ATTRIBUTE_REQUEST:
				CavanMessage("Get print attribute request");
				getPrinterAttribute();
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
		Log.v("Cavan", message);
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

	private boolean listBluetoothDevices()
	{
		mTextViewStatus.setText("list bluetooth devices");

		mBluetoothDevice = null;

		if (mBluetoothAdapter == null)
		{
			showAlertDialogYes("getDefaultAdapter failed");
			return false;
		}

		if (mBluetoothAdapter.isEnabled() == false)
		{
			showAlertDialogYes("BluetoothAdapter is disabled");
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

		if (radioButton == null)
		{
			showAlertDialogYes("No bluetooth device is paired");
		}

		return true;
	}

	public void StreamCopy(InputStream inputStream, OutputStream outputStream) throws IOException
	{
		byte[] buff = new byte[1024];
		int readlen;

		while (true)
		{
			readlen = inputStream.read(buff);
			if (readlen < 0)
			{
				break;
			}

			outputStream.write(buff, 0, readlen);
		}
	}

	private void readBinaryFromAssets(AssetManager manager, String parent, final String command) throws IOException
	{
		Log.v(TAG, "command = " + command);

		InputStream inputStream = manager.open("bin/" + command);
		File file = new File(parent + "/" + command);
		if (inputStream.available() == file.length())
		{
			Log.v(TAG, "file is exist, continue");
			return;
		}
		FileOutputStream outputStream = new FileOutputStream(file);
		StreamCopy(inputStream, outputStream);
		inputStream.close();
		outputStream.close();
		file.setExecutable(true);
	}

	private void readBinaryFromAssets() throws IOException
	{
		Context context = getApplicationContext();
		AssetManager manager = context.getAssets();
		String parent = context.getDir("bin", Context.MODE_PRIVATE).getPath();

		mBinaryFilePdfToJpeg = parent + "/pdf2jpeg";
		readBinaryFromAssets(manager, parent, "pdf2jpeg");
	}

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		try
		{
			readBinaryFromAssets();
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}

		Intent service = new Intent(this, BluetoothPrintService.class);
		bindService(service, mServiceConnection, Context.BIND_AUTO_CREATE);

		mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

		mButtonJobBasePrint = (Button) findViewById(R.id.main_button_job_print);
		mButtonJobBasePrint.setOnClickListener(mOnClickListener);

		mButtonSimplePushPrint = (Button) findViewById(R.id.main_button_simple_print);
		mButtonSimplePushPrint.setOnClickListener(mOnClickListener);

		mButtonRefresh = (Button) findViewById(R.id.main_button_refresh);
		mButtonRefresh.setOnClickListener(mOnClickListener);

		mButtonPrintScan = (Button) findViewById(R.id.main_button_printer_scan);
		mButtonPrintScan.setOnClickListener(mOnClickListener);

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

	public BluetoothPrintJob getPrintJob()
	{
		return mPrintJob;
	}

	public BluetoothDevice getBluetoothDevice()
	{
		return mBluetoothDevice;
	}

	public Handler getHandler()
	{
		return mHandler;
	}

	public File Pdf2Jpeg(String pdfpath, int page, String jpgname) throws IOException, InterruptedException
	{
		File file = new File(getCacheDir() + "/" + jpgname);
		FileOutputStream outputStream = new FileOutputStream(file);

		String command = String.format("%s %s %d", mBinaryFilePdfToJpeg, pdfpath, page);
		Log.v(TAG, "command = " + command);

		Process process = Runtime.getRuntime().exec(command);
		InputStream inputStream = process.getInputStream();

		StreamCopy(inputStream, outputStream);
		inputStream.close();
		outputStream.close();

		process.waitFor();
		if (process.exitValue() != 0)
		{
			Log.v(TAG, "process.exitValue() = " + process.exitValue());
			return null;
		}

		return file;
	}

	public int getPdfPageCount(String pdfpath) throws IOException, InterruptedException
	{
		String command = String.format("%s %s", mBinaryFilePdfToJpeg, pdfpath);
		Log.v(TAG, "command = " + command);
		Process process = Runtime.getRuntime().exec(command);
		process.waitFor();
		if (process.exitValue() != 0)
		{
			return -1;
		}

		InputStream inputStream = process.getInputStream();
		int length = inputStream.available();
		if (length <= 0)
		{
			Log.e(TAG, "inputStream.available() = " + length);
			return -1;
		}

		byte[] buff = new byte[length];
		inputStream.read(buff);

		return Integer.decode(new String(buff));
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