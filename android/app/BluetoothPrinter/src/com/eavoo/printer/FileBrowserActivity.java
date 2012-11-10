package com.eavoo.printer;

import java.io.File;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;

public class FileBrowserActivity extends Activity
{
	private ListView mListViewBrowser;
	private FileBowserAdapter mFileBowserAdapter;

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
		setTitle(file.getAbsolutePath());
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

		mListViewBrowser = (ListView) findViewById(R.id.browser_listView_files);

		mFileBowserAdapter = new FileBowserAdapter(this, getIntent().getData());
		mListViewBrowser.setAdapter(mFileBowserAdapter);
		mListViewBrowser.setOnItemClickListener(mOnItemClickListener);
		showPath();
	}
}
