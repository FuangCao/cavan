package com.cavan.location;

import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Set;

import android.app.Activity;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.ArraySet;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBusyLock;
import com.cavan.cavanmain.R;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanMessageQueue;
import com.cavan.java.CavanMessageQueue.CavanMessage;
import com.cavan.java.CavanTcpPacketClient;

public class MockLocationActivity extends Activity implements OnClickListener {

	private static final int MSG_CONNECT = 1;
	private static final int MSG_DISCONNECT = 2;

	private EditText mEditTextLatitude;
	private EditText mEditTextLatitudeStep;
	private EditText mEditTextLongitude;
	private EditText mEditTextLongitudeStep;

	private Button mButtonAdd;
	private Button mButtonEdit;
	private Button mButtonCollect;
	private Button mButtonOpen;
	private Button mButtonDisconn;
	private Button mButtonLatitudeAdd;
	private Button mButtonLatitudeSub;
	private Button mButtonLongitudeAdd;
	private Button mButtonLongitudeSub;

	private ListView mListViewServers;
	private ArrayList<LocationClient> mClients = new ArrayList<LocationClient>();
	private LinkedList<LocationClient> mFreeClients = new LinkedList<LocationClient>();

	private double mLatitude = 31.133196;
	private double mLatitudeStep = 0.0001;
	private double mLongitude = 121.39552;
	private double mLongitudeStep = 0.0001;

	private BaseAdapter mAdapter = new BaseAdapter() {

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			LocationServiceView view;

			if (convertView != null) {
				view = (LocationServiceView) convertView;
			} else {
				view = LocationServiceView.getInstance(MockLocationActivity.this);
			}

			LocationClient client = mClients.get(position);
			view.setLocationClient(client);

			return view;
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public Object getItem(int position) {
			return mClients.get(position);
		}

		@Override
		public int getCount() {
			return mClients.size();
		}
	};

	private Runnable mRunnableNotifyDataSetChanged = new Runnable() {

		@Override
		public void run() {
			mAdapter.notifyDataSetChanged();
		}
	};

	private CavanBusyLock mBusyLock = new CavanBusyLock(300);

	private CavanMessageQueue mMessageQueue = new CavanMessageQueue() {

		@Override
		protected void handleMessage(CavanMessage message) {
			LocationClient client = (LocationClient) message.args[0];

			switch (message.what) {
			case MSG_CONNECT:
				client.connect(message.getString(1), message.getInt(2));
				break;

			case MSG_DISCONNECT:
				client.disconnect();
				break;
			}
		}
	};

	private Runnable mRunnableSendLocation = new Runnable() {

		@Override
		public void run() {
			CavanAndroid.dLog("mLatitude = " + mLatitude);
			CavanAndroid.dLog("Longitude = " + mLongitude);

			StringBuilder builder = new StringBuilder();
			builder.append("set_location ");
			builder.append(mLatitude).append(' ').append(mLongitude);

			String command = builder.toString();

			CavanAndroid.dfLog("%.6f %.6f", mLatitude, mLongitude);

			CavanAndroid.dLog("command = " + command);

			for (LocationClient client : mClients) {
				client.send(command);
			}
		}
	};

	private Runnable mRunnableDisconn = new Runnable() {

		@Override
		public void run() {
			for (LocationClient client : mClients) {
				client.disconnect();
			}
		}
	};

	private void sendLocation() {
		mMessageQueue.postReplace(mRunnableSendLocation);
	}

	public void connect(LocationClient client, String host, int port) {
		CavanMessage message = mMessageQueue.obtainMessage(MSG_CONNECT, client, host, port);
		message.send();
	}

	public void disconnect(LocationClient client) {
		CavanMessage message = mMessageQueue.obtainMessage(MSG_DISCONNECT, client);
		message.send();
	}

	private SharedPreferences getSharedPreferences() {
		return getSharedPreferences("mock_location", 0);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_mock_location);

		SharedPreferences preferences = getSharedPreferences();

		mEditTextLatitude = (EditText) findViewById(R.id.editTextLatitude);
		mEditTextLatitudeStep = (EditText) findViewById(R.id.editTextLatitudeStep);
		mEditTextLongitude = (EditText) findViewById(R.id.editTextLongitude);
		mEditTextLongitudeStep = (EditText) findViewById(R.id.editTextLongitudeStep);

		mButtonAdd = (Button) findViewById(R.id.buttonAdd);
		mButtonEdit = (Button) findViewById(R.id.buttonEdit);
		mButtonCollect = (Button) findViewById(R.id.buttonCollect);
		mButtonOpen = (Button) findViewById(R.id.buttonOpen);
		mButtonDisconn = (Button) findViewById(R.id.buttonDisconn);
		mButtonLatitudeAdd = (Button) findViewById(R.id.buttonLatitudeAdd);
		mButtonLatitudeSub = (Button) findViewById(R.id.buttonLatitudeSub);
		mButtonLongitudeAdd = (Button) findViewById(R.id.buttonLongitudeAdd);
		mButtonLongitudeSub = (Button) findViewById(R.id.buttonLongitudeSub);

		mListViewServers = (ListView) findViewById(R.id.listViewServers);
		mListViewServers.setAdapter(mAdapter);

		String text = preferences.getString("latitude", null);
		if (text != null) {
			mLatitude = CavanJava.parseDouble(text);
		}

		text = preferences.getString("longitude", null);
		if (text != null) {
			mLongitude = CavanJava.parseDouble(text);
		}

		mEditTextLatitude.setText(Double.toString(mLatitude));
		mEditTextLongitude.setText(Double.toString(mLongitude));

		mLatitudeStep = CavanJava.parseDouble(mEditTextLatitudeStep.getText().toString());
		mLongitudeStep = CavanJava.parseDouble(mEditTextLongitudeStep.getText().toString());

		mEditTextLatitudeStep.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void afterTextChanged(Editable s) {
				mLatitudeStep = CavanJava.parseDouble(s.toString());
			}
		});

		mEditTextLongitudeStep.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void afterTextChanged(Editable s) {
				mLongitudeStep = CavanJava.parseDouble(s.toString());
			}
		});

		mButtonAdd.setOnClickListener(this);
		mButtonEdit.setOnClickListener(this);
		mButtonCollect.setOnClickListener(this);
		mButtonOpen.setOnClickListener(this);
		mButtonDisconn.setOnClickListener(this);
		mButtonLatitudeAdd.setOnClickListener(this);
		mButtonLatitudeSub.setOnClickListener(this);
		mButtonLongitudeAdd.setOnClickListener(this);
		mButtonLongitudeSub.setOnClickListener(this);

		Set<String> servers = preferences.getStringSet("servers", null);
		if (servers != null) {
			for (String server : servers) {
				String[] args = server.split("\\s*:\\s*");
				if (args.length != 2) {
					continue;
				}

				try {
					String host = args[0];
					int port = Integer.parseInt(args[1]);
					LocationClient client = getFreeClient();

					synchronized (mClients) {
						mClients.add(client);
					}

					connect(client, host, port);
				} catch (NumberFormatException e) {
					e.printStackTrace();
				}
			}
		}

		synchronized (mClients) {
			if (mClients.isEmpty()) {
				mClients.add(getFreeClient());
			}
		}
	}

	@Override
	protected void onDestroy() {
		try {
			ArraySet<String> servers = new ArraySet<String>();

			synchronized (mClients) {
				for (LocationClient client : mClients) {
					InetSocketAddress address = client.getFirstAddress();
					if (address == null) {
						continue;
					}

					disconnect(client);
					servers.add(address.getHostString() + ":" + address.getPort());
				}

				mClients.clear();
			}

			Editor editor = getSharedPreferences().edit();
			editor.putString("latitude", Double.toString(mLatitude));
			editor.putString("longitude", Double.toString(mLongitude));
			editor.putStringSet("servers", servers);
			editor.commit();
		} catch (Exception e) {
			e.printStackTrace();
		}

		super.onDestroy();
	}

	private LocationClient getFreeClient() {
		synchronized (mFreeClients) {
			try {
				LocationClient client = mFreeClients.pop();
				if (client != null) {
					return client;
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		return new LocationClient();
	}

	@Override
	public void onClick(View v) {
		if (v == mButtonAdd) {
			LocationClient client = getFreeClient();

			synchronized (mClients) {
				mClients.add(client);
				mListViewServers.post(mRunnableNotifyDataSetChanged);
			}
		} else if (v == mButtonDisconn) {
			mMessageQueue.postReplace(mRunnableDisconn);
		} else if (v == mButtonLatitudeAdd) {
			mLatitude += mLatitudeStep;
			sendLocation();
			mBusyLock.acquire(v);
			mEditTextLatitude.setEnabled(false);
			mEditTextLatitude.setText(Double.toString(mLatitude));
		} else if (v == mButtonLatitudeSub) {
			mLatitude -= mLatitudeStep;
			sendLocation();
			mBusyLock.acquire(v);
			mEditTextLatitude.setEnabled(false);
			mEditTextLatitude.setText(Double.toString(mLatitude));
		} else if (v == mButtonLongitudeAdd) {
			mLongitude += mLongitudeStep;
			sendLocation();
			mBusyLock.acquire(v);
			mEditTextLongitude.setEnabled(false);
			mEditTextLongitude.setText(Double.toString(mLongitude));
		} else if (v == mButtonLongitudeSub) {
			mLongitude -= mLongitudeStep;
			sendLocation();
			mBusyLock.acquire(v);
			mEditTextLongitude.setEnabled(false);
			mEditTextLongitude.setText(Double.toString(mLongitude));
		} else if (v == mButtonEdit) {
			if (mEditTextLatitude.isEnabled() || mEditTextLongitude.isEnabled()) {
				mEditTextLatitude.setEnabled(false);
				mEditTextLongitude.setEnabled(false);
				mLatitude = CavanJava.parseDouble(mEditTextLatitude.getText().toString());
				mLongitude = CavanJava.parseDouble(mEditTextLongitude.getText().toString());
				sendLocation();
			} else {
				mEditTextLatitude.setEnabled(true);
				mEditTextLongitude.setEnabled(true);
			}
		} else if (v == mButtonCollect) {

		} else if (v == mButtonOpen) {

		}
	}

	public void removeLocationClient(LocationClient client) {
		disconnect(client);

		synchronized (mClients) {
			mClients.remove(client);
		}

		synchronized (mFreeClients) {
			mFreeClients.push(client);
		}

		mListViewServers.post(mRunnableNotifyDataSetChanged);
	}

	public class LocationClient extends CavanTcpPacketClient {

		private boolean mIsBusy;

		public void setBusy(boolean busy) {
			mIsBusy = busy;
		}

		public boolean isBusy() {
			return mIsBusy;
		}

		@Override
		protected boolean onPacketReceived(byte[] bytes, int length) {
			CavanAndroid.dLog("onPacketReceived: " + length);
			return true;
		}

		@Override
		protected boolean onTcpConnected(Socket socket) {
			mListViewServers.post(mRunnableNotifyDataSetChanged);
			sendLocation();
			return true;
		}

		@Override
		protected void onTcpDisconnected() {
			mListViewServers.post(mRunnableNotifyDataSetChanged);
		}

		@Override
		protected void onTcpClientRunning() {
			mIsBusy = false;
			mListViewServers.post(mRunnableNotifyDataSetChanged);
		}

		@Override
		protected void onTcpClientStopped() {
			mIsBusy = false;
			mListViewServers.post(mRunnableNotifyDataSetChanged);
		}

		@Override
		public void prErrInfo(String message) {
			CavanAndroid.eLog(message);
		}

		@Override
		public void prWarnInfo(String message) {
			CavanAndroid.dLog(message);
		}

		@Override
		public void prDbgInfo(String message) {
			CavanAndroid.dLog(message);
		}
	}
}
