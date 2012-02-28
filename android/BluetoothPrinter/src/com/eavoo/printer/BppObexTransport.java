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
	private static final UUID mUuidOBEXObjectPush = UUID.fromString("00001123-0000-1000-8000-00805F9B34FB");

	private UUID mServerUuid;
    private BluetoothSocket mBluetoothSocket;
    private boolean mConnected = false;
	private BluetoothDevice mBluetoothDevice;

    public BppObexTransport(BluetoothDevice device, UUID uuid)
    {
    	this.mBluetoothDevice = device;
        this.mServerUuid = uuid;
    }

    public BppObexTransport(BluetoothDevice device)
    {
    	this(device, mUuidOBEXObjectPush);
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

    public void connect() throws IOException
    {
    	mBluetoothSocket = mBluetoothDevice.createInsecureRfcommSocketToServiceRecord(mServerUuid);
    	mBluetoothSocket.connect();
    	mConnected = true;
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
