package com.cavan.bluetoothinterphone;

import java.io.InputStream;
import java.io.OutputStream;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder.AudioSource;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanDaemonThread;

public class InterphoneAudioDevice {

	private static final int SAMPLE_RATE = 8000;
	private static final int FORMAT = AudioFormat.ENCODING_PCM_16BIT;
	private static final int CHANNEL_IN = AudioFormat.CHANNEL_IN_MONO;
	private static final int CHANNEL_OUT = AudioFormat.CHANNEL_OUT_MONO;

	public void record(CavanDaemonThread thread, OutputStream stream) {
		CavanAndroid.pLog();

		AudioRecord record = null;

		try {
			int size = AudioRecord.getMinBufferSize(SAMPLE_RATE, CHANNEL_IN, FORMAT);

			record = new AudioRecord(AudioSource.MIC, SAMPLE_RATE, CHANNEL_IN, FORMAT, size * 2);
			record.startRecording();

			byte[] bytes = new byte[size];

			while (true) {
				if (thread.isSuspended()) {
					break;
				}

				int length = record.read(bytes, 0, bytes.length);
				if (length < 0) {
					break;
				}

				stream.write(bytes, 0, length);
			}
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			if (record != null) {
				try {
					record.stop();
				} catch (Exception e) {
					e.printStackTrace();
				}

				record.release();
			}
		}
	}

	public void play(CavanDaemonThread thread, InputStream stream) {
		CavanAndroid.pLog();

		AudioTrack track = null;

		try {
			int size = AudioTrack.getMinBufferSize(SAMPLE_RATE, CHANNEL_OUT, FORMAT);

			track = new AudioTrack(AudioManager.STREAM_MUSIC, SAMPLE_RATE, CHANNEL_OUT, FORMAT, size * 2, AudioTrack.MODE_STREAM);
			track.play();

			byte[] bytes = new byte[size];

			while (true) {
				if (thread.isSuspended()) {
					break;
				}

				int length = stream.read(bytes);
				if (length < 0) {
					break;
				}

				track.write(bytes, 0, length);
			}
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			if (track != null) {
				try {
					track.stop();
				} catch (Exception e) {
					e.printStackTrace();
				}

				track.release();
			}
		}
	}
}
