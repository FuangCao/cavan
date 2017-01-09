package com.cavan.java;

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
import java.io.RandomAccessFile;
import java.io.Reader;
import java.io.Writer;
import java.net.URI;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel.MapMode;
import java.util.ArrayList;
import java.util.List;

public class CavanFile extends File {

	public static final String NEW_LINE_DOS = "\r\n";
	public static final String NEW_LINE_UNIX = "\n";
	public static final String NEW_LINE_DEFAULT = NEW_LINE_UNIX;
	private static final long serialVersionUID = 2944296431050993672L;

	private String mNewLine;

	public interface CavanReplaceHandler {
		public String replace(String text);
	}

	public interface CavanScanHandler {
		public boolean scan(String line);
	}

	class CavanReadLinesHandler implements CavanScanHandler {

		protected List<String> mLines;

		public CavanReadLinesHandler(List<String> lines) {
			mLines = lines;
		}

		@Override
		public boolean scan(String text) {
			mLines.add(text);
			return false;
		}
	}

	class CavanReplaceLinesHandler extends CavanReadLinesHandler {

		protected CavanReplaceHandler mHandler;

		public CavanReplaceLinesHandler(List<String> lines, CavanReplaceHandler handler) {
			super(lines);
			mHandler = handler;
		}

		@Override
		public boolean scan(String text) {
			return super.scan(mHandler.replace(text));
		}
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

	public FileInputStream openInputStream() {
		try {
			return new FileInputStream(this);
		} catch (FileNotFoundException e) {
			return null;
		}
	}

	public FileOutputStream openOutputStream() {
		try {
			return new FileOutputStream(this);
		} catch (FileNotFoundException e) {
			return null;
		}
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
			return null;
		}
	}

	public FileWriter openWriter(boolean append) {
		try {
			return new FileWriter(this, append);
		} catch (IOException e) {
			return null;
		}
	}

	public BufferedReader openBufferedReader() {
		Reader reader = openReader();
		if (reader == null) {
			return null;
		}

		return new BufferedReader(reader);
	}

	public BufferedWriter openBufferedWriter(boolean append) {
		Writer writer = openWriter(append);
		if (writer == null) {
			return null;
		}

		return new BufferedWriter(writer);
	}

	public RandomAccessFile openRandomAccessFile(String mode) {
		try {
			return new RandomAccessFile(this, mode);
		} catch (FileNotFoundException e) {
			return null;
		}
	}

	public MappedByteBuffer mmap(long position, long size, boolean readonly) {
		RandomAccessFile access = openRandomAccessFile("rw");
		if (access == null) {
			return null;
		}

		try {
			if (size < 0) {
				size = access.length();
			}

			return access.getChannel().map(readonly ? MapMode.READ_ONLY : MapMode.READ_WRITE, position, size);
		} catch (IOException e) {
			e.printStackTrace();
		}

		try {
			access.close();
		} catch (IOException e) {
			e.printStackTrace();
		}

		return null;
	}

	private String findNewLine() {
		InputStream stream = openInputStream();
		if (stream != null) {
			byte[] bytes = new byte[1024];
			boolean rfound = false;

			try {
				while (true) {
					int length = stream.read(bytes);
					if (length <= 0) {
						if (length < 0) {
							break;
						}

						continue;
					}

					for (int i = 0; i < length; i++) {
						switch (bytes[i]) {
						case '\r':
							rfound = true;
							break;

						case '\n':
							if (rfound) {
								return NEW_LINE_DOS;
							}
							break;

						default:
							rfound = false;
						}
					}
				}
			} catch (IOException e) {
				e.printStackTrace();
			} finally {
				try {
					stream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}

		return NEW_LINE_DEFAULT;
	}

	synchronized public String getNewLine() {
		if (mNewLine == null) {
			mNewLine = findNewLine();
		}

		return mNewLine;
	}

	synchronized public void setNewLine(String newLine) {
		mNewLine = newLine;
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
			return -1;
		} finally {
			try {
				stream.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
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
			CavanJava.ArrayCopy(bytes, newBytes, length);
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

	public String readText(CavanReplaceHandler handler) {
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
			stream.flush();
			return count;
		} catch (IOException e) {
			e.printStackTrace();
			return -1;
		} finally {
			try {
				stream.close();
			} catch (IOException e) {
				e.printStackTrace();
				return -1;
			}
		}
	}

	public int write(byte[] bytes, int count) {
		return write(bytes, 0, count);
	}

	public int write(byte[] bytes) {
		return write(bytes, bytes.length);
	}

	public int writeText(String text, int offset, int count, boolean append) {
		Writer writer = openWriter(append);
		if (writer == null) {
			return -1;
		}

		try {
			writer.write(text, offset, count);
			writer.flush();
			return count;
		} catch (IOException e) {
			e.printStackTrace();
			return -1;
		} finally {
			try {
				writer.close();
			} catch (IOException e) {
				e.printStackTrace();
				return -1;
			}
		}
	}

	public int writeText(String text, int offset, int count) {
		return writeText(text, offset, count, false);
	}

	public int writeText(String text, int count) {
		return writeText(text, 0, count);
	}

	public int writeText(String text) {
		return writeText(text, 0, text.length());
	}

	public int appendText(String text, int offset, int count) {
		return writeText(text, offset, count, true);
	}

	public int appendText(String text, int count) {
		return appendText(text, 0, count);
	}

	public int appendText(String text) {
		return appendText(text, 0, text.length());
	}

	public boolean writeLines(List<String> lines, String newLine, boolean append) {
		Writer writer = openWriter(append);
		if (writer == null) {
			return false;
		}

		try {
			for (String line : lines) {
				writer.write(line);
				writer.write(newLine);
			}

			writer.flush();

			return true;
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		} finally {
			try {
				writer.close();
			} catch (IOException e) {
				e.printStackTrace();
				return false;
			}
		}
	}

	public boolean writeLines(List<String> lines, boolean append) {
		return writeLines(lines, getNewLine(), append);
	}

	public boolean writeLinesDos(List<String> lines, boolean append) {
		return writeLines(lines, NEW_LINE_DOS, append);
	}

	public boolean writeLinesUnix(List<String> lines, boolean append) {
		return writeLines(lines, NEW_LINE_UNIX, append);
	}

	public boolean rewriteLines(CavanFile dest, String newLine, boolean append) {
		List<String> lines = readLines();
		if (lines == null) {
			return false;
		}

		if (dest == null) {
			dest = this;
		}

		return dest.writeLines(lines, newLine, append);
	}

	public boolean rewriteLines(String newLine, boolean append) {
		return rewriteLines(this, newLine, append);
	}

	public boolean writeDos2Unix(CavanFile dest, boolean append) {
		return rewriteLines(dest, NEW_LINE_UNIX, append);
	}

	public boolean writeDos2Unix(boolean append) {
		return writeDos2Unix(this, append);
	}

	public boolean writeDos2Unix() {
		return writeDos2Unix(false);
	}

	public boolean writeUnix2Dos(CavanFile dest, boolean append) {
		return rewriteLines(dest, NEW_LINE_DOS, append);
	}

	public boolean writeUnix2Dos(boolean append) {
		return writeUnix2Dos(append);
	}

	public boolean writeUnix2Dos() {
		return writeUnix2Dos(false);
	}

	public boolean scanLines(CavanScanHandler handler) {
		BufferedReader reader = openBufferedReader();
		if (reader == null) {
			return false;
		}

		try {
			while (true) {
				String line = reader.readLine();
				if (line == null) {
					break;
				}

				if (handler.scan(line)) {
					break;
				}
			}

			return true;
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		} finally {
			try {
				reader.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	public boolean readLines(List<String> lines) {
		return scanLines(new CavanReadLinesHandler(lines));
	}

	public List<String> readLines() {
		List<String> lines = new ArrayList<String>();
		if (readLines(lines)) {
			return lines;
		}

		return null;
	}

	public boolean readLines(CavanReplaceHandler handler, List<String> lines) {
		CavanReplaceLinesHandler scanHandler = new CavanReplaceLinesHandler(lines, handler);
		return scanLines(scanHandler);
	}

	public List<String> readLines(CavanReplaceHandler handler) {
		List<String> lines = new ArrayList<String>();
		if (readLines(handler, lines)) {
			return lines;
		}

		return null;
	}

	public boolean replaceLines(CavanReplaceHandler handler, CavanFile dest) {
		List<String> lines = readLines(handler);
		if (lines == null) {
			return false;
		}

		if (dest == null) {
			dest = this;
		}

		return dest.writeLines(lines, getNewLine(), false);
	}

	public boolean replaceLines(CavanReplaceHandler handler) {
		return replaceLines(handler, this);
	}

	public boolean replaceText(CavanReplaceHandler handler, CavanFile dest) {
		String text = readText(handler);
		if (text == null) {
			return false;
		}

		if (dest == null) {
			dest = this;
		}

		return dest.write(text.getBytes()) > 0;
	}

	public boolean replaceText(CavanReplaceHandler handler) {
		return replaceText(handler, this);
	}

	public static boolean deleteDir(File dir, boolean subOnly) {
		for (File file : dir.listFiles()) {
			if (file.isDirectory()) {
				if (!deleteDir(file, false)) {
					return false;
				}
			} else if (!file.delete()) {
				return false;
			}
		}

		if (subOnly) {
			return true;
		}

		return dir.delete();
	}

	public static boolean deleteAll(File file, boolean subOnly) {
		if (file.isDirectory()) {
			return deleteDir(file, subOnly);
		}

		if (file.delete()) {
			return true;
		}

		return !file.exists();
	}

	public boolean deleteAll(boolean subOnly) {
		return deleteAll(this, subOnly);
	}

	public boolean deleteAll() {
		return deleteAll(this, false);
	}

	public boolean clear() {
		if (isDirectory()) {
			return deleteAll(true);
		}

		if (exists()) {
			return delete();
		}

		return true;
	}

	public static boolean copy(File inFile, File outFile) {
		if (inFile.isDirectory()) {
			if (!outFile.isDirectory()) {
				if (outFile.exists()) {
					return false;
				}

				if (!outFile.mkdir()) {
					return false;
				}
			}

			for (String name : inFile.list()) {
				return copy(new File(inFile, name), new File(outFile, name));
			}
		} else if (outFile.isDirectory()) {
			outFile = new File(outFile, inFile.getName());
		}

		FileInputStream inStream = null;
		FileOutputStream outStream = null;

		try {
			inStream = new FileInputStream(inFile);
			outStream = new FileOutputStream(outFile);

			MappedByteBuffer buffer = inStream.getChannel().map(MapMode.READ_ONLY, 0, inStream.available());
			if (buffer == null) {
				byte[] bytes = new byte[1024];

				while (true) {
					int length = inStream.read(bytes);
					if (length <= 0) {
						if (length < 0) {
							break;
						}

						continue;
					}

					outStream.write(bytes, 0, length);
				}
			} else {
				outStream.getChannel().write(buffer);
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

	public static boolean copy(String inPath, String outPath) {
		return copy(new File(inPath), new File(outPath));
	}

	public boolean copyFrom(File file) {
		return copy(file, this);
	}

	public boolean copyFrom(String pathname) {
		return copyFrom(new File(pathname));
	}

	public boolean copyTo(File file) {
		return copy(this, file);
	}

	public boolean copyTo(String pathname) {
		return copyTo(new File(pathname));
	}

	public boolean mkdirSafe() {
		return mkdir() || isDirectory();
	}

	public boolean mkdirsSafe() {
		return mkdirs() || isDirectory();
	}

	public boolean isNewThen(File file) {
		return lastModified() > file.lastModified();
	}

	public boolean isNewOrEqualThen(File file) {
		return lastModified() >= file.lastModified();
	}

	public boolean isOldThen(File file) {
		return lastModified() < file.lastModified();
	}

	public boolean isOldOrEqualThen(File file) {
		return lastModified() <= file.lastModified();
	}

	public static String getMimeType(String pathname) {
		CavanCommand command = new CavanCommand( "file", "-b", "--mime-type", pathname );
		List<String> lines = command.doPipe();
		if (lines != null && lines.size() > 0) {
			return lines.get(0);
		}

		return null;
	}

	public String getMimeType() {
		return getMimeType(getPath());
	}

	public static boolean isDotName(String name) {
		if (name.charAt(0) != '.') {
			return false;
		}

		int length = name.length();
		if (length == 1) {
			return true;
		}

		return length == 2 && name.charAt(1) == '.';
	}

	public static String getName(String abspath) {
		String[] paths = abspath.split(separator);
		int index = paths.length - 1;

		while (index >= 0) {
			String name = paths[index];

			if (name.equals(".")) {
				index--;
			} else if (name.equals("..")) {
				index -= 2;
			} else {
				return name;
			}
		}

		return "/";
	}

	@Override
	public String getName() {
		String name = super.getName();
		if (isDotName(name)) {
			try {
				String pathname = getCanonicalPath();
				int index = pathname.lastIndexOf(separatorChar);
				if (index >= 0) {
					name = pathname.substring(index + 1);
				}
			} catch (IOException e) {
				name = getName(getAbsolutePath());
			}
		}

		return name;
	}
}