package com.cavan.cavanutils;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.Reader;
import java.io.Writer;
import java.net.URI;
import java.util.ArrayList;
import java.util.List;

public class CavanFile extends File {

	private static final long serialVersionUID = 2944296431050993672L;

	public CavanFile(File dir, String name) {
		super(dir, name);
	}

	public CavanFile(String dirPath, String name) {
		super(dirPath, name);
	}

	public CavanFile(String path) {
		super(path);
	}

	public CavanFile(URI uri) {
		super(uri);
	}

	public InputStream openInputStream() {
		try {
			return new FileInputStream(this);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}

		return null;
	}

	public OutputStream openOutputStream() {
		try {
			return new FileOutputStream(this);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}

		return null;
	}

	public BufferedInputStream openBufferedInputStream() {
		InputStream stream = openInputStream();
		if (stream == null) {
			return null;
		}

		return new BufferedInputStream(stream);
	}

	public BufferedOutputStream openBufferedOutputStream() {
		OutputStream stream = openOutputStream();
		if (stream == null) {
			return null;
		}

		return new BufferedOutputStream(stream);
	}

	public Reader openReader() {
		try {
			return new FileReader(this);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}

		return null;
	}

	public Writer openWriter() {
		try {
			return new FileWriter(this);
		} catch (IOException e) {
			e.printStackTrace();
		}

		return null;
	}

	public BufferedReader openBufferedReader() {
		Reader reader = openReader();
		if (reader == null) {
			return null;
		}

		return new BufferedReader(reader);
	}

	public BufferedWriter openBufferedWriter() {
		Writer writer = openWriter();
		if (writer == null) {
			return null;
		}

		return new BufferedWriter(writer);
	}

	public int read(byte[] bytes, int skip, int offset, int count) {
		InputStream stream = openInputStream();
		if (stream == null) {
			return -1;
		}

		try {
			if (skip > 0) {
				stream.skip(skip);
			}

			if (count < 0) {
				count = stream.available();
			}

			return stream.read(bytes, offset, count);
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			try {
				stream.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		return -1;
	}

	public byte[] read(int count) {
		if (count < 0) {
			count = (int) length();
		}

		byte[] bytes = new byte[count];
		int length = read(bytes, 0, 0, count);
		if (length < 0) {
			return null;
		}

		if (length != count) {
			byte[] newBytes = new byte[length];
			CavanUtils.ArrayCopy(bytes, newBytes, length);
			return newBytes;
		}

		return bytes;
	}

	public String readText() {
		byte[] bytes = read(-1);
		if (bytes == null) {
			return null;
		}

		return new String(bytes);
	}

	public int write(byte[] bytes, int offset, int count) {
		OutputStream stream = openOutputStream();
		if (stream == null) {
			return -1;
		}

		try {
			stream.write(bytes, offset, count);
			return count;
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			try {
				stream.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		return -1;
	}

	public List<String> readlines() {
		BufferedReader reader = openBufferedReader();
		if (reader == null) {
			return null;
		}

		List<String> lines = new ArrayList<String>();
		while (true) {
			try {
				String line = reader.readLine();
				if (line == null) {
					break;
				}

				lines.add(line);
			} catch (IOException e) {
				e.printStackTrace();
				lines = null;
				break;
			}
		}

		try {
			reader.close();
		} catch (IOException e) {
			e.printStackTrace();
		}

		return lines;
	}
}
