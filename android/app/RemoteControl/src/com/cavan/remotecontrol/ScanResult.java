package com.cavan.remotecontrol;

import java.net.InetAddress;
import android.annotation.SuppressLint;
import android.os.Parcel;
import android.os.Parcelable;

@SuppressLint("DefaultLocale")
public class ScanResult implements Parcelable {
	private int mPort;
	private String mHostname;
	private InetAddress mAddress;

	public ScanResult(int port, String hostname, InetAddress address) {
		super();
		mPort = port;
		mHostname = hostname;
		mAddress = address;
	}

	public int getPort() {
		return mPort;
	}

	public void setPort(int port) {
		mPort = port;
	}

	public String getHostname() {
		return mHostname;
	}

	public void setHostname(String hostname) {
		mHostname = hostname;
	}

	public InetAddress getAddress() {
		return mAddress;
	}

	public void setAddress(InetAddress address) {
		mAddress = address;
	}

	@Override
	public String toString() {
		return String.format("%s:%d - %s", mAddress.getHostAddress(), mPort, mHostname);
	}

	public String toShortString() {
		return String.format("%s - %s", mAddress.getHostAddress(), mHostname);
	}

	@Override
	public int describeContents() {
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags) {
		dest.writeInt(mPort);
		dest.writeString(mHostname);
		dest.writeSerializable(mAddress);
	}

	public static final Creator<ScanResult> CREATOR = new Creator<ScanResult>() {

		@Override
		public ScanResult[] newArray(int size) {
			return new ScanResult[size];
		}

		@Override
		public ScanResult createFromParcel(Parcel source) {
			return new ScanResult(source.readInt(), source.readString(), (InetAddress) source.readSerializable());
		}
	};
}
