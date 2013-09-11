package com.cavan.touchscreen;

import java.io.File;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;

public class FileBrowserActivity extends Activity
{
	private static final String TAG = "Cavan";

	private ListView mListViewBrowser;
	private FileBrowserAdapter mFileBrowserAdapter;
	public static final String PATHNAME_CHANGED_ACTION = "com.cavan.huatpupgrade.PATHNAME_CHANGED";

	private OnItemClickListener mOnItemClickListener = new OnItemClickListener()
	{
		@Override
		public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3)
		{
			File file = (File) mFileBrowserAdapter.getItem(arg2);
			if (file.isDirectory())
			{
				mFileBrowserAdapter.setCurrentDirectory(file);
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
		setTitle(file.getAbsolutePath());
	}

	private void showPath()
	{
		showPath(mFileBrowserAdapter.getCurrentDirectory());
	}

	private void finish(String pathname)
	{
		Intent intent = new Intent(PATHNAME_CHANGED_ACTION);
		intent.putExtra("pathname", pathname);
		sendBroadcast(intent);
		finish();
	}

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.browser);

		mListViewBrowser = (ListView) findViewById(R.id.browser_listView_files);

		mFileBrowserAdapter = new FileBrowserAdapter(this, getIntent().getStringExtra("pathname"));
		mListViewBrowser.setAdapter(mFileBrowserAdapter);
		mListViewBrowser.setOnItemClickListener(mOnItemClickListener);
		showPath();
	}
}
