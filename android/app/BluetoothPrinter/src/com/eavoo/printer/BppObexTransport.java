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
	public static final UUID uuidDirectPrinting  = UUID.fromString("00001118-0000-1000-8000-00805F9B34FB");
	public static final UUID uuidReferencePrinting = UUID.fromString("00001119-0000-1000-8000-00805F9B34FB");
	public static final UUID uuidDirectPrintingReferenceObjectsService = UUID.fromString("00001120-0000-1000-8000-00805F9B34FB");
	public static final UUID uuidReflectedUI = UUID.fromString("00001121-0000-1000-8000-00805F9B34FB");
	public static final UUID uuidPrintingStatus = UUID.fromString("00001123-0000-1000-8000-00805F9B34FB");

	private static UUID mPrinterUuid;
	private BluetoothSocket mBluetoothSocket;
	private boolean mConnected = false;
	private BluetoothDevice mBluetoothDevice;

	public BppObexTransport(BluetoothDevice device, UUID uuid)
	{
		this.mBluetoothDevice = device;
		BppObexTransport.mPrinterUuid = uuid;
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

	private UUID connect(UUID... uuids)
	{
		for (UUID uuid : uuids)
		{
			if (uuid == null)
			{
				continue;
			}

			try
			{
				mBluetoothSocket = mBluetoothDevice.createInsecureRfcommSocketToServiceRecord(uuid);
				mBluetoothSocket.connect();
				return uuid;
			}
			catch (IOException e)
			{
				e.printStackTrace();
			}
		}

		return null;
	}

	public void connect() throws IOException
	{
		UUID uuid = connect(mPrinterUuid, uuidDirectPrinting, uuidPrintingStatus, uuidReferencePrinting, uuidReflectedUI);
		if (uuid == null)
		{
			mConnected = false;
			throw new IOException();
		}

		mPrinterUuid = uuid;
		mConnected = true;
	}

	public UUID getPrinterUuid()
	{
		return mPrinterUuid;
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
