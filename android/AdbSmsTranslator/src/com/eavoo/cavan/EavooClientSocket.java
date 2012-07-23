package com.eavoo.cavan;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

public class EavooClientSocket
{
	public static final int SMS_TYPE_END = 0x00;
	public static final int SMS_TYPE_TEST = 0x01;
	public static final int SMS_TYPE_ACK = 0x02;
	public static final int SMS_TYPE_OKAY = 0x03;
	public static final int SMS_TYPE_FAILED = 0x04;
	public static final int SMS_TYPE_DATE = 0x05;
	public static final int SMS_TYPE_PHONE = 0x06;
	public static final int SMS_TYPE_CONTENT = 0x07;

	private Socket mSocket;
	private InputStream mInputStream;
	private OutputStream mOutputStream;

	public Socket getmSocket()
	{
		return mSocket;
	}

	public InputStream getInputStream()
	{
		return mInputStream;
	}

	public OutputStream getOutputStream()
	{
		return mOutputStream;
	}

	public EavooClientSocket(Socket socket) throws IOException
	{
		mOutputStream = socket.getOutputStream();
		mInputStream = socket.getInputStream();
		mSocket = socket;
	}

	@Override
	protected void finalize() throws Throwable
	{
		close();
		super.finalize();
	}

	public boolean close()
	{
		if (mSocket.isClosed())
		{
			return true;
		}

		boolean result = true;

		try
		{
			mInputStream.close();
			mSocket.shutdownInput();
		}
		catch (IOException e)
		{
			// e.printStackTrace();
			result = false;
		}

		try
		{
			mOutputStream.close();
			mSocket.shutdownOutput();
		}
		catch (IOException e)
		{
			// e.printStackTrace();
			result = false;
		}

		try
		{
			mSocket.close();
		}
		catch (IOException e)
		{
			// e.printStackTrace();
			result = false;
		}

		return result;
	}

	public int write(byte[] buff)
	{
		try
		{
			mOutputStream.write(buff);
			return mInputStream.read();
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}

		return -1;
	}

	public int write(EavooShortMessage message)
	{
		try
		{
			mOutputStream.write(message.toByteArray());
			return mInputStream.read();
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}

		return -1;
	}

	public int test_network()
	{
		try
		{
			mOutputStream.write(SMS_TYPE_TEST);
			return mInputStream.read();
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}

		return -1;
	}
}
