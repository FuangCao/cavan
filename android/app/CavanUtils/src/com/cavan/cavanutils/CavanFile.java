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

	interface ReplaceHandler {
		public String replace(String text);
	}

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

	public String readText(ReplaceHandler handler) {
		String text = readText();
		if (text == null) {
			return null;
		}

		return handler.replace(text);
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

	public int write(byte[] bytes, int count) {
		return write(bytes, 0, count);
	}

	public int write(byte[] bytes) {
		return write(bytes, bytes.length);
	}

	public boolean writeLines(List<String> lines, String newLine) {
		OutputStream stream = openOutputStream();
		if (stream == null) {
			return false;
		}

		try {
			for (String line : lines) {
				stream.write(line.getBytes());
				stream.write(newLine.getBytes());
			}

			return true;
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		} finally {
			try {
				stream.close();
			} catch (IOException e) {
				e.printStackTrace();
				return false;
			}
		}
	}

	public boolean writeLines(List<String> lines) {
		return writeLines(lines, "\n");
	}

	public List<String> readLines(ReplaceHandler handler) {
		BufferedReader reader = openBufferedReader();
		if (reader == null) {
			return null;
		}

		List<String> lines = new ArrayList<String>();

		try {
			if (handler != null) {
				while (true) {
					String line = reader.readLine();
					if (line == null) {
						break;
					}

					line = handler.replace(line);
					if (line != null) {
						lines.add(line);
					}
				}
			} else {
				while (true) {
					String line = reader.readLine();
					if (line == null) {
						break;
					}

					lines.add(line);
				}
			}

			return lines;
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		} finally {
			try {
				reader.close();
			} catch (IOException e) {
				e.printStackTrace();
				return null;
			}
		}
	}

	public boolean replaceLines(ReplaceHandler handler, CavanFile fileSave) {
		List<String> lines = readLines(handler);
		if (lines == null) {
			return false;
		}

		return fileSave.writeLines(lines);
	}

	public boolean replaceLines(ReplaceHandler handler) {
		return replaceLines(handler, this);
	}

	public boolean replaceText(ReplaceHandler handler, CavanFile fileSave) {
		String text = readText(handler);
		if (text == null) {
			return false;
		}

		return fileSave.write(text.getBytes()) > 0;
	}

	public boolean replaceText(ReplaceHandler handler) {
		return replaceText(handler, this);
	}

	public static boolean deleteDir(File dir) {
		for (File file : dir.listFiles()) {
			if (file.isDirectory()) {
				if (!deleteDir(file)) {
					return false;
				}
			} else if (!file.delete()) {
				return false;
			}
		}

		return dir.delete();
	}

	public static boolean deleteAll(File file) {
		if (file.isDirectory()) {
			return deleteDir(file);
		}

		if (file.delete()) {
			return true;
		}

		return !file.exists();
	}

	public boolean deleteAll() {
		return deleteAll(this);
	}

	public static boolean copy(File inFile, File outFile) {
		InputStream inStream = null;
		OutputStream outStream = null;

		try {
			inStream = new FileInputStream(inFile);
			outStream = new FileOutputStream(outFile);
			byte[] bytes = new byte[1024];

			while (true) {
				int length = inStream.read(bytes);
				if (length < 0) {
					break;
				}

				outStream.write(bytes, 0, length);
			}

			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		} finally {
			if (inStream != null) {
				try {
					inStream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}

			if (outStream != null) {
				try {
					outStream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}

	public boolean copyFrom(File file) {
		return copy(file, this);
	}

	public boolean copyTo(File file) {
		return copy(this, file);
	}

	public boolean mkdirSafe() {
		return mkdir() || isDirectory();
	}

	public boolean mkdirsSafe() {
		return mkdirs() || isDirectory();
	}
}
