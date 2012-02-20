package com.eavoo.printer;

import java.io.File;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

public class FileBrowserActivity extends Activity
{
	private static final String mRootDirectory = "/sdcard";
	private ListView mListViewBrowser;
	private FileBowserAdapter mFileBowserAdapter;
	private TextView mTextViewPath;
	private Button mButtonBack;
	private Button mButtonSelect;

	private OnClickListener mOnClickListener = new OnClickListener()
	{
		@Override
		public void onClick(View v)
		{
			switch (v.getId())
			{
			case R.id.browser_button1:
				finish(mTextViewPath.getText().toString());
				break;

			case R.id.browser_button2:
				mFileBowserAdapter.backParentDirectory();
				showPath();
				break;

			default:
				Log.e("Cavan", "unknown onClick id");
			}
		}
	};

	private OnItemClickListener mOnItemClickListener = new OnItemClickListener()
	{
		@Override
		public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3)
		{
			File file = (File) mFileBowserAdapter.getItem(arg2);
			if (file.isDirectory())
			{
				mFileBowserAdapter.setCurrentDirectory(file);
				showPath(file);
			}
			else
			{
				finish(file.getAbsolutePath());
			}
		}
	};

	private void showPath(File file)
	{
		mTextViewPath.setText(file.getAbsolutePath());
	}

	private void showPath()
	{
		showPath(mFileBowserAdapter.getCurrentDirectory());
	}

	private void finish(String pathname)
	{
		Intent intent = getIntent().putExtra("pathname", pathname);
		setResult(0, intent);
		finish();
	}

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.browser);

		mTextViewPath = (TextView) findViewById(R.id.browser_textView1);
		mButtonSelect = (Button) findViewById(R.id.browser_button1);
		mButtonBack = (Button) findViewById(R.id.browser_button2);
		mListViewBrowser = (ListView) findViewById(R.id.browser_listView1);

		mFileBowserAdapter = new FileBowserAdapter(this, mRootDirectory);
		mListViewBrowser.setAdapter(mFileBowserAdapter);
		mListViewBrowser.setOnItemClickListener(mOnItemClickListener);
		showPath();
		mButtonBack.setOnClickListener(mOnClickListener);
		mButtonSelect.setOnClickListener(mOnClickListener);
	}
}
