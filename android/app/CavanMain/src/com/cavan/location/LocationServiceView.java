package com.cavan.location;

import java.net.InetSocketAddress;

import android.content.Context;
import android.graphics.Color;
import android.util.AttributeSet;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RelativeLayout;

import com.cavan.cavanmain.R;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanTcpPacketClient;
import com.cavan.location.MockLocationActivity.LocationClient;

public class LocationServiceView extends RelativeLayout implements OnClickListener {

	private EditText mEditTextHost;
	private EditText mEditTextPort;
	private Button mButtonConn;
	private Button mButtonDel;

	private LocationClient mLocationClient;
	private MockLocationActivity mActivity;

	public LocationServiceView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
	}

	public LocationServiceView(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public LocationServiceView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public LocationServiceView(Context context) {
		super(context);
	}

	private void init(MockLocationActivity activity) {
		mActivity = activity;

		mEditTextHost = (EditText) findViewById(R.id.editTextHost);
		mEditTextPort = (EditText) findViewById(R.id.editTextPort);
		mButtonConn = (Button) findViewById(R.id.buttonConn);
		mButtonDel = (Button) findViewById(R.id.buttonDel);

		mButtonConn.setOnClickListener(this);
		mButtonDel.setOnClickListener(this);
	}

	public CavanTcpPacketClient getTcpClient() {
		return mLocationClient;
	}

	public void setLocationClient(LocationClient client) {
		if (mLocationClient != client) {
			mLocationClient = client;

			InetSocketAddress address = client.getFirstAddress();
			if (address != null) {
				mEditTextHost.setText(address.getHostString());
				mEditTextPort.setText(Integer.toString(address.getPort()));
			} else {
				int port = CavanJava.parseInt(mEditTextPort.getText().toString());
				String host = mEditTextHost.getText().toString();
				client.setAddress(host, port);
			}
		}

		if (client.isConnDisabled()) {
			mButtonConn.setText(R.string.connect);
			mButtonConn.setTextColor(Color.BLACK);
		} else {
			mButtonConn.setText(R.string.disconnect);

			if (client.isConnected()) {
				mButtonConn.setTextColor(Color.GREEN);
			} else {
				mButtonConn.setTextColor(Color.RED);
			}
		}

		mButtonConn.setEnabled(!client.isBusy());
	}

	public boolean sendCommand(String command) {
		return mLocationClient.send(command);
	}

	public static LocationServiceView getInstance(MockLocationActivity activity) {
		LocationServiceView view = (LocationServiceView) View.inflate(activity, R.layout.service_item, null);
		view.init(activity);
		return view;
	}

	@Override
	public void onClick(View v) {
		if (v == mButtonConn) {
			try {
				mButtonConn.setEnabled(false);
				mLocationClient.setBusy(true);

				if (mLocationClient.isConnDisabled()) {
					int port = CavanJava.parseInt(mEditTextPort.getText().toString());
					String host = mEditTextHost.getText().toString();
					mActivity.connect(mLocationClient, host, port);
				} else {
					mActivity.disconnect(mLocationClient);
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		} else if (v == mButtonDel) {
			mActivity.removeLocationClient(mLocationClient);
		}
	}
}