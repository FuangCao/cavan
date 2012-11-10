package com.eavoo.touchscreen;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import android.content.Context;
import android.net.Uri;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

public class FileBrowserAdapter extends BaseAdapter
{
	private static final String mRootDirectory = "/";
	private static final String mDefaultDirectory = "/sdcard";

	private File mCurrentDirectory;
	private File mParentDirectory;
	private List<File> mFileList;
	private Context mContext;

	public FileBrowserAdapter(Context context, Uri uri)
	{
		this.mContext = context;

		String currPath = null;
		if (uri != null)
		{
			currPath = uri.getPath();
		}

		if (currPath == null || currPath.isEmpty())
		{
			mCurrentDirectory = new File(mDefaultDirectory);
		}
		else
		{
			mCurrentDirectory = new File(currPath);
			if (mCurrentDirectory.isDirectory() == false)
			{
				mCurrentDirectory = mCurrentDirectory.getParentFile();
			}
		}

		setCurrentDirectory(mCurrentDirectory);
	}

	public File getCurrentDirectory()
	{
		return mCurrentDirectory;
	}

	public void setCurrentDirectory(File directory)
	{
		mFileList = new ArrayList<File>();
		File[] files = directory.listFiles();
		if (files != null)
		{
			for (File file : files)
			{
				mFileList.add(file);
			}
		}

		Collections.sort(mFileList, new Comparator<File>()
		{
			@Override
			public int compare(File lhs, File rhs)
			{
				if (lhs.isDirectory())
				{
					return rhs.isDirectory() ? 0 : 1;
				}

				return rhs.isDirectory() ? -1 : 0;
			}
		});

		mCurrentDirectory = directory;
		mParentDirectory = mCurrentDirectory.getParentFile();
		if (mParentDirectory != null)
		{
			mFileList.add(0, mParentDirectory);
		}

		notifyDataSetInvalidated();
	}

	public boolean backParentDirectory()
	{
		if (mCurrentDirectory.getAbsolutePath().equals(mRootDirectory))
		{
			return false;
		}

		setCurrentDirectory(mCurrentDirectory.getParentFile());

		return true;
	}

	@Override
	public int getCount()
	{
		return mFileList.size();
	}

	@Override
	public Object getItem(int arg0)
	{
		return mFileList.get(arg0);
	}

	@Override
	public long getItemId(int position)
	{
		return position;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent)
	{
		File file = mFileList.get(position);
		if (file == null)
		{
			return null;
		}

		LinearLayout layout = new LinearLayout(mContext);

		ImageView imageView = new ImageView(mContext);
		LayoutParams params = new LayoutParams(50, 50);
		imageView.setLayoutParams(params);
		layout.addView(imageView);

		TextView textView = new TextView(mContext);
		layout.addView(textView);

		if (file == mParentDirectory)
		{
			imageView.setImageResource(R.drawable.arrowup);
			textView.setText(file.getAbsolutePath());
		}
		else if (file.isDirectory())
		{
			imageView.setImageResource(R.drawable.folderopen);
			textView.setText(file.getName());
		}
		else
		{
			imageView.setImageResource(R.drawable.marienkaefer);
			textView.setText(file.getName());
		}

		return layout;
	}
}
