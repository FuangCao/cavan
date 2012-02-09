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
import android.util.Log;

public class BppObexTransport implements ObexTransport
{
	private static final UUID mUuidOBEXObjectPush = UUID.fromString("00001123-0000-1000-8000-00805F9B34FB");

	public static final byte[] UUID_DPS =
	{
		0x00, 0x00, 0x11, 0x18, 0x00, 0x00, 0x10, 0x00,
		(byte) 0x80, 0x00, 0x00, (byte) 0x80, 0x5F, (byte) 0x9B, 0x34, (byte) 0xFB
	};

	public static final byte[] UUID_PBR =
	{
		0x00, 0x00, 0x11, 0x19, 0x00, 0x00, 0x10, 0x00,
		(byte) 0x80, 0x00, 0x00, (byte) 0x80, 0x5F, (byte) 0x9B, 0x34, (byte) 0xFB
	};

	public static final byte[] UUID_REF_OBJ =
	{
		0x00, 0x00, 0x11, 0x20, 0x00, 0x00, 0x10, 0x00,
		(byte) 0x80, 0x00, 0x00, (byte) 0x80, 0x5F, (byte) 0x9B, 0x34, (byte) 0xFB
	};

	public static final byte[] UUID_URI_REF_OBJ =
	{
		0x00, 0x00, 0x11, 0x21, 0x00, 0x00, 0x10, 0x00,
		(byte) 0x80, 0x00, 0x00, (byte) 0x80, 0x5F, (byte) 0x9B, 0x34, (byte) 0xFB
	};

	public static final byte[] UUID_STS =
	{
		0x00, 0x00, 0x11, 0x23, 0x00, 0x00, 0x10, 0x00,
		(byte) 0x80, 0x00, 0x00, (byte) 0x80, 0x5F, (byte) 0x9B, 0x34, (byte) 0xFB
	};

	private UUID mServerUuid;
    private BluetoothSocket mBluetoothSocket;
    private boolean mConnected = false;
	private BluetoothDevice mBluetoothDevice;

	private void CavanLog(String message)
	{
		Log.v("Cavan", "\033[1m" + message + "\033[0m");
	}

    public BppObexTransport(BluetoothDevice device, UUID uuid)
    {
    	CavanLog("Create BppObexTransport");
    	this.mBluetoothDevice = device;
        this.mServerUuid = uuid;
    }

    public BppObexTransport(BluetoothDevice device)
    {
    	this(device, mUuidOBEXObjectPush);
    }

    public void close() throws IOException
    {
    	CavanLog("close");
    	mBluetoothSocket.close();
    }

    public DataInputStream openDataInputStream() throws IOException
    {
    	CavanLog("openDataInputStream");
        return new DataInputStream(openInputStream());
    }

    public DataOutputStream openDataOutputStream() throws IOException
    {
    	CavanLog("openDataOutputStream");
        return new DataOutputStream(openOutputStream());
    }

    public InputStream openInputStream() throws IOException
    {
    	CavanLog("openInputStream");
        return mBluetoothSocket.getInputStream();
    }

    public OutputStream openOutputStream() throws IOException
    {
    	CavanLog("openOutputStream");
        return mBluetoothSocket.getOutputStream();
    }

    public void connect() throws IOException
    {
        // TODO Auto-generated method stub
    	CavanLog("connect");
    	mBluetoothSocket = mBluetoothDevice.createInsecureRfcommSocketToServiceRecord(mServerUuid);
    	mBluetoothSocket.connect();
    	mConnected = true;
    }

    public void create() throws IOException
    {
        // TODO Auto-generated method stub
    	CavanLog("create");
    }

    public void disconnect() throws IOException
    {
        // TODO Auto-generated method stub
    	CavanLog("disconnect");
    }

    public void listen() throws IOException
    {
        // TODO Auto-generated method stub
    	CavanLog("listen");
    }

    public boolean isConnected() throws IOException
    {
    	CavanLog("isConnected");
        return mConnected;
    }
}
