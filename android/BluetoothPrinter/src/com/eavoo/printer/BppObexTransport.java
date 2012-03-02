package com.eavoo.printer;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

import javax.obex.ObexTransport;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;

public class BppObexTransport implements ObexTransport
{
	private static final UUID[] mUuidsBPP =
	{
		UUID.fromString("00001118-0000-1000-8000-00805F9B34FB"),
		UUID.fromString("00001123-0000-1000-8000-00805F9B34FB"),
		UUID.fromString("00001119-0000-1000-8000-00805F9B34FB"),
		UUID.fromString("00001120-0000-1000-8000-00805F9B34FB"),
		UUID.fromString("00001121-0000-1000-8000-00805F9B34FB"),
	};

	private static UUID mServerUuid;
	private BluetoothSocket mBluetoothSocket;
	private boolean mConnected = false;
	private BluetoothDevice mBluetoothDevice;

	public BppObexTransport(BluetoothDevice device, UUID uuid)
	{
		this.mBluetoothDevice = device;
		BppObexTransport.mServerUuid = uuid;
	}

	public BppObexTransport(BluetoothDevice device)
	{
		this.mBluetoothDevice = device;
	}

	public void close() throws IOException
	{
		mBluetoothSocket.close();
	}

	public DataInputStream openDataInputStream() throws IOException
	{
		return new DataInputStream(openInputStream());
	}

	public DataOutputStream openDataOutputStream() throws IOException
	{
		return new DataOutputStream(openOutputStream());
	}

	public InputStream openInputStream() throws IOException
	{
		return mBluetoothSocket.getInputStream();
	}

	public OutputStream openOutputStream() throws IOException
	{
		return mBluetoothSocket.getOutputStream();
	}

	private boolean connect(UUID uuid)
	{
		try
		{
			mBluetoothSocket = mBluetoothDevice.createInsecureRfcommSocketToServiceRecord(uuid);
			mBluetoothSocket.connect();
			return true;
		}
		catch (IOException e) {}

		return false;
	}

	public void connect() throws IOException
	{
		if (mServerUuid != null)
		{
			mConnected = connect(mServerUuid);
			if (mConnected)
			{
				return;
			}
		}

		for (UUID uuid : mUuidsBPP)
		{
			mConnected = connect(uuid);
			if (mConnected)
			{
				mServerUuid = uuid;
				return;
			}
		}

		mConnected = false;
		throw new IOException();
	}

	public void create() throws IOException
	{
	}

	public void disconnect() throws IOException
	{
	}

	public void listen() throws IOException
	{
	}

	public boolean isConnected() throws IOException
	{
		return mConnected;
	}
}
