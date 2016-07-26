package com.cavan.testhttp;

import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.DefaultHttpClient;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

public class MainActivity extends Activity {

	private static final String TAG = "Cavan";

	// public static final String TEST_URL = "http://192.168.1.19:5688";
	// public static final String TEST_URL = "http://www.jd.com";
	// public static final String TEST_URL = "https://s3-us-west-2.amazonaws.com/kevinbucket1/105025.mp4";
	public static final String TEST_URL = "http://192.168.1.19:5678/kevinbucket1/105025.mp4";

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		new HttpURLConnectionThread().start();
		// new OkHttpThread().start();
	}

	public boolean showInputStream(String name, InputStream stream) {
		byte[] bytes = new byte[102400];
		long total = 0;
		int max = 0;

		try {
			Log.e(TAG, "available = " + stream.available());
		} catch (IOException e1) {
			e1.printStackTrace();
		}

		while (true) {
			int length;

			try {
				length = stream.read(bytes);
			} catch (IOException e) {
				e.printStackTrace();
				return false;
			}

			Log.e(TAG, "length = " + length + ", total = " + total);
			if (length <= 0) {
				break;
			}

			if (length > max) {
				max = length;
			}

			total += length;
			// Log.e(TAG, "content = " + new String(bytes, 0, length));
		}

		Log.e(TAG, name + ": max = " + max + ", total = " + total);

		return true;
	}

	public class OkHttpThread extends Thread {

		@Override
		public void run() {
			OkHttpClient client = new OkHttpClient();
			Request request = new Request.Builder().url(TEST_URL).build();
			try {
				Response response = client.newCall(request).execute();
				if (response.isSuccessful()) {
					InputStream stream = response.body().byteStream();
					showInputStream(getClass().getName(), stream);
					stream.close();
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	public class HttpURLConnectionThread extends Thread {

		@Override
		public void run() {
			try {
				URL url = new URL(TEST_URL);
				HttpURLConnection connection = (HttpURLConnection) url.openConnection();
				int response = connection.getResponseCode();
				Log.e(TAG, "response = " + response);
				if (response == 200) {
					InputStream stream = connection.getInputStream();
					showInputStream(getClass().getName(), stream);
					stream.close();
				}

				connection.disconnect();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	public class HttpClientThread extends Thread {

		@Override
		public void run() {
			HttpGet req = new HttpGet(TEST_URL);
			HttpClient client = new DefaultHttpClient();

			try {
				HttpResponse response = client.execute(req);
				HttpEntity entity = response.getEntity();
				InputStream stream = entity.getContent();
				showInputStream(getClass().getName(), stream);
				stream.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
}
