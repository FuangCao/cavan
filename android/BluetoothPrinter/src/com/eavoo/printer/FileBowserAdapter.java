package com.eavoo.printer;

import java.io.File;
import java.lang.reflect.Array;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import android.content.Context;
import android.sax.RootElement;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

public class FileBowserAdapter extends BaseAdapter
{
	private String mRootDirectory;
	private File mCurrentDirectory;
	private List<File> mFileList;
	private Context mContext;

	public FileBowserAdapter(Context context, String rootPath)
	{
		this.mContext = context;
		this.mRootDirectory = rootPath;

		this.mCurrentDirectory = new File(mRootDirectory);
		setCurrentDirectory(mCurrentDirectory);
	}

	public File getCurrentDirectory()
	{
		return mCurrentDirectory;
	}

	public void setCurrentDirectory(File directory)
	{
		File[] files = directory.listFiles();
		if (files == null)
		{
			mFileList = Collections.emptyList();
		}
		else
		{
			mFileList = Arrays.asList(files);
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

		this.mCurrentDirectory = directory;
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
		// TODO Auto-generated method stub
		return mFileList.size();
	}

	@Override
	public Object getItem(int arg0)
	{
		// TODO Auto-generated method stub
		return mFileList.get(arg0);
	}

	@Override
	public long getItemId(int position)
	{
		// TODO Auto-generated method stub
		return position;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent)
	{
		// TODO Auto-generated method stub
		File file = mFileList.get(position);

		LinearLayout layout = new LinearLayout(mContext);
		ImageView imageView = new ImageView(mContext);

		if (file.isDirectory())
		{
			imageView.setImageResource(R.drawable.folderopen);
		}
		else
		{
			imageView.setImageResource(R.drawable.book);
		}
		layout.addView(imageView);

		TextView textView = new TextView(mContext);
		textView.setText(file.getName());
		layout.addView(textView);

		return layout;
	}
}
