package com.cavan.cavanplayer;

import java.io.IOException;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnTimedTextListener;
import android.media.MediaPlayer.OnVideoSizeChangedListener;
import android.media.TimedText;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;

public class MainActivity extends ActionBarActivity {

	private static final String TAG = "Cavan";
	
@SuppressLint("NewApi")
class PlayerSurfaceView extends SurfaceView implements Callback, OnVideoSizeChangedListener, OnTimedTextListener, OnCompletionListener {

		private SurfaceHolder mHolder;
		private MediaPlayer mPlayer = new MediaPlayer();

		public PlayerSurfaceView(Context context) {
			super(context);
			setSystemUiVisibility(SYSTEM_UI_FLAG_FULLSCREEN | SYSTEM_UI_FLAG_HIDE_NAVIGATION | STATUS_BAR_HIDDEN);

			mHolder = getHolder();
			mHolder.addCallback(this);
		}

		@Override
		public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		}

		@Override
		public void surfaceCreated(SurfaceHolder holder) {
			mPlayer.reset();
			mPlayer.setDisplay(mHolder);
			mPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
			mPlayer.setOnVideoSizeChangedListener(this);
			mPlayer.setOnTimedTextListener(this);
			mPlayer.setOnCompletionListener(this);

			try {
				Intent intent = getIntent();
				mPlayer.setDataSource(getApplicationContext(), intent.getData());
				mPlayer.prepare();
				mPlayer.start();
			} catch (IllegalArgumentException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (SecurityException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IllegalStateException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		@Override
		public void surfaceDestroyed(SurfaceHolder holder) {
			if (mPlayer.isPlaying()) {
				mPlayer.stop();
			}
		}

		@Override
		public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {
			Log.w(TAG, "onVideoSizeChanged: width = " + width + ", height = " + height, new Throwable());

			mHolder.setFixedSize(mp.getVideoWidth(), mp.getVideoHeight());
		}

		@Override
		public void onCompletion(MediaPlayer mp) {
			finish();
		}

		@Override
		public void onTimedText(MediaPlayer arg0, TimedText arg1) {
			Log.d(TAG, "onTimedText: text = " + arg1);
		}
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		int flags = WindowManager.LayoutParams.FLAG_FULLSCREEN | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON;
		getWindow().setFlags(flags, flags);
		requestWindowFeature(Window.FEATURE_NO_TITLE);

		setContentView(new PlayerSurfaceView(this));
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
}
