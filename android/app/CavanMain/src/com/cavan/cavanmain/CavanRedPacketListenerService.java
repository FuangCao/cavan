package com.cavan.cavanmain;

import java.io.IOException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.app.Notification;
import android.content.ClipboardManager;
import android.content.res.AssetFileDescriptor;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.service.notification.NotificationListenerService;
import android.service.notification.StatusBarNotification;

import com.cavan.android.CavanAndroid;

public class CavanRedPacketListenerService extends NotificationListenerService {

	public static final Pattern mPatternZFB = Pattern.compile("支付宝.*口令\\D*(\\d+)");

	private ClipboardManager mClipboard;
	private AssetFileDescriptor mAudioFileNotify;

	@Override
	public void onCreate() {
		mClipboard = (ClipboardManager) getApplication().getSystemService(CLIPBOARD_SERVICE);

		try {
			mAudioFileNotify = getAssets().openFd("notify.m4a");
		} catch (IOException e) {
			e.printStackTrace();
		}

		super.onCreate();
	}

	@SuppressWarnings("deprecation")
	@Override
	public void onNotificationPosted(StatusBarNotification sbn) {
		Notification notification = sbn.getNotification();
		Bundle extras = notification.extras;
		CharSequence content = extras.getCharSequence(Notification.EXTRA_TEXT);

		Matcher matcher = mPatternZFB.matcher(content);
		if (matcher.find()) {
			String code = matcher.group(1);

			CavanAndroid.logD("code = " + code);
			mClipboard.setText(code);
			
			MediaPlayer player = new MediaPlayer();
			try {
				player.setDataSource(mAudioFileNotify.getFileDescriptor(), mAudioFileNotify.getStartOffset(), mAudioFileNotify.getLength());
				player.setAudioStreamType(AudioManager.STREAM_NOTIFICATION);
				player.prepare();
				player.start();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}

		super.onNotificationPosted(sbn);
	}
}
