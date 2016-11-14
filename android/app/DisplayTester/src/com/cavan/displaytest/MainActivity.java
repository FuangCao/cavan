package com.cavan.displaytest;

import android.app.Activity;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;

public class MainActivity extends Activity {

	private final int COLOR_LIST[] = { 
        Color.RED, Color.GREEN, Color.BLUE,
        Color.GREEN | Color.BLUE, Color.RED | Color.BLUE, Color.RED | Color.GREEN,
        Color.BLACK, Color.GRAY, Color.WHITE
    };

	private int mIndex;
	private SurfaceView mSurfaceView;
	private SurfaceHolder mSurfaceHolder;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		mSurfaceView = new SurfaceView(this);
		mSurfaceHolder = mSurfaceView.getHolder();
		mSurfaceHolder.setFormat(PixelFormat.RGBA_8888);

		setContentView(mSurfaceView);
	}

	private void drawColor(int color) {
		Canvas canvas = mSurfaceHolder.lockCanvas();
		canvas.drawColor(color);
		mSurfaceHolder.unlockCanvasAndPost(canvas);
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		if (event.getAction() == MotionEvent.ACTION_DOWN) {
			if (mIndex >= COLOR_LIST.length) {
				mIndex = 0;
			}

			drawColor(COLOR_LIST[mIndex]);

			mIndex++;
		}

		return super.onTouchEvent(event);
	}
}
