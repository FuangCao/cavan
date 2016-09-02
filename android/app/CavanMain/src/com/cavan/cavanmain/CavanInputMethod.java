package com.cavan.cavanmain;

import java.util.List;

import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.inputmethodservice.InputMethodService;
import android.os.IBinder;
import android.os.RemoteException;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;

public class CavanInputMethod extends InputMethodService implements OnClickListener, OnLongClickListener {

	private ListView mListView;
	private List<String> mCodes;

	private BaseAdapter mAdapter = new BaseAdapter() {

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			Button button = new Button(CavanInputMethod.this);
			button.setOnClickListener(CavanInputMethod.this);
			button.setOnLongClickListener(CavanInputMethod.this);
			button.setText(mCodes.get(position));
			return button;
		}

		@Override
		public long getItemId(int position) {
			return 0;
		}

		@Override
		public Object getItem(int position) {
			return null;
		}

		@Override
		public int getCount() {
			if (mCodes != null) {
				return mCodes.size();
			}

			return 0;
		}
	};

	private IFloatMessageService mService;
	private ServiceConnection mConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName name) {
			mService = null;
		}

		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			mService = IFloatMessageService.Stub.asInterface(service);
			updateData();
		}
	};

	public void updateData() {
		if (mService != null) {
			try {
				mCodes = mService.getCodes();
				mAdapter.notifyDataSetChanged();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}
	}

	public void sendRedPacketCode(CharSequence text) {
		InputConnection conn = getCurrentInputConnection();
		conn.performContextMenuAction(android.R.id.selectAll);
		conn.commitText(text, 0);
		conn.performEditorAction(EditorInfo.IME_ACTION_GO);
	}

	@Override
	public void onCreate() {
		mListView = new ListView(this);
		mListView.setAdapter(mAdapter);

		Intent service = FloatMessageService.startService(this);
		bindService(service, mConnection, 0);

		super.onCreate();
	}

	@Override
	public void onDestroy() {
		unbindService(mConnection);
		super.onDestroy();
	}

	@Override
	public void onStartInput(EditorInfo attribute, boolean restarting) {
		updateData();

		if (mCodes == null || mCodes.size() == 0) {
			InputConnection conn = getCurrentInputConnection();
			conn.performContextMenuAction(android.R.id.switchInputMethod);
		}

		super.onStartInput(attribute, restarting);
	}

	@Override
	public View onCreateInputView() {
		return mListView;
	}

	@Override
	public void onClick(View v) {
		Button button = (Button) v;
		sendRedPacketCode(button.getText());
	}

	@Override
	public boolean onLongClick(View v) {
		InputConnection conn = getCurrentInputConnection();
		conn.performContextMenuAction(android.R.id.switchInputMethod);
		return true;
	}
}
