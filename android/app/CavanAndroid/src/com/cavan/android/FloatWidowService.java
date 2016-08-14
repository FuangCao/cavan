package com.cavan.android;

import java.util.HashMap;

import android.app.Service;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.os.IBinder;
import android.os.RemoteException;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.widget.LinearLayout;

public abstract class FloatWidowService extends Service {

	protected ViewGroup mLayout;
	protected WindowManager mManager;
	protected HashMap<Integer, View> mViewMap = new HashMap<Integer, View>();

	private IFloatWindowService.Stub mBinder = new IFloatWindowService.Stub() {

		@Override
		public int addText(CharSequence text, int index) throws RemoteException {
			View view = FloatWidowService.this.addText(text, index);
			if (view != null) {
				return view.getId();
			}

			return -1;
		}

		@Override
		public void removeText(CharSequence text) throws RemoteException {
			FloatWidowService.this.removeView(text);
		}

		@Override
		public void removeTextAt(int index) throws RemoteException {
			FloatWidowService.this.removeView(index);
		}

		@Override
		public void removeTextById(int id) throws RemoteException {
			FloatWidowService.this.removeView(id);

		}

		@Override
		public void removeAll() throws RemoteException {
			FloatWidowService.this.removeAll();
		}
	};

	protected abstract CharSequence getViewText(View view);
	protected abstract View createView(CharSequence text);

	synchronized public int addView(View view, int index) {
		for (int id = 1; id > 0; id++) {
			if (mViewMap.get(id) == null) {
				mViewMap.put(id, view);
				view.setId(id);

				if (index < 0) {
					mLayout.addView(view);
				} else {
					mLayout.addView(view, index);
				}

				return id;
			}
		}

		return -1;
	}

	public View addText(CharSequence text, int index) {
		View view = createView(text);
		if (view != null && addView(view, index) < 0) {
			view = null;
		}

		return view;
	}

	synchronized public View findView(int id) {
		return mViewMap.get(id);
	}

	synchronized public View findView(CharSequence text) {
		for (View view : mViewMap.values()) {
			if (text.equals(getViewText(view))) {
				return view;
			}
		}

		return null;
	}

	synchronized public View getViewAt(int index) {
		try {
			return mLayout.getChildAt(index);
		} catch (Exception e) {
			return null;
		}
	}

	synchronized public void removeView(View view) {
		mLayout.removeView(view);
		mViewMap.remove(view.getId());
	}

	synchronized public void removeView(int id) {
		View view = findView(id);
		if (view != null) {
			removeView(view);
		}
	}

	synchronized public void removeView(CharSequence text) {
		View view = findView(text);
		if (view != null) {
			removeView(view);
		}
	}

	synchronized public void removeViewAt(int index) {
		View view = getViewAt(index);
		if (view != null) {
			removeView(view);
		}
	}

	synchronized public void removeAll() {
		mLayout.removeAllViews();
		mViewMap.clear();
	}

	protected LayoutParams createLayoutParams() {
		LayoutParams params = new LayoutParams();

		params.x = params.y = 0;
		params.type = LayoutParams.TYPE_PHONE;
		params.format = PixelFormat.RGBA_8888;
		params.gravity = Gravity.RIGHT | Gravity.TOP;
		params.width = WindowManager.LayoutParams.WRAP_CONTENT;
		params.height = WindowManager.LayoutParams.WRAP_CONTENT;
		params.flags = LayoutParams.FLAG_NOT_FOCUSABLE | LayoutParams.FLAG_NOT_TOUCHABLE;

		return params;
	}

	protected ViewGroup createLayout() {
		return new LinearLayout(getApplicationContext());
	}

	@Override
	public void onCreate() {
		mManager = (WindowManager) getApplicationContext().getSystemService(WINDOW_SERVICE);
		if (mManager != null) {
			mLayout = createLayout();
			if (mLayout != null) {
				mManager.addView(mLayout, createLayoutParams());
			}
		}

		super.onCreate();
	}

	@Override
	public void onDestroy() {
		removeAll();
		mManager.removeView(mLayout);

		super.onDestroy();
	}

	@Override
	public IBinder onBind(Intent arg0) {
		return mBinder;
	}
}
