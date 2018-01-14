package com.cavan.weixinredpacket;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.graphics.PixelFormat;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.widget.Button;

public class CountDownDialog implements OnClickListener, Runnable {

	private Context mContext;

	private View mRootView;
	private Button mButtonNow;
	private Button mButtonCancel;

	private Dialog mDialog;
	private long mCommitTime;
	private long mDismissTime;

	public CountDownDialog(Context context) {
		mContext = context;

		mRootView = View.inflate(context, R.layout.count_down_dialog, null);

		mButtonCancel = (Button) findViewById(R.id.buttonCancel);
		mButtonCancel.setOnClickListener(this);

		mButtonNow = (Button) findViewById(R.id.buttonNow);
		mButtonNow.setOnClickListener(this);
	}

	private View findViewById(int id) {
		return mRootView.findViewById(id);
	}

	private LayoutParams createLayoutParams(int flags) {
		LayoutParams params = new LayoutParams(
				WindowManager.LayoutParams.WRAP_CONTENT,
				WindowManager.LayoutParams.WRAP_CONTENT,
				LayoutParams.TYPE_SYSTEM_ALERT,
				flags,
				PixelFormat.RGBA_8888);

		return params;
	}

	public void show(long time) {
		mCommitTime = time;

		if (mDialog == null) {
			AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
			builder.setView(mRootView);
			builder.setCancelable(true);
			mDialog = builder.create();
			mDialog.getWindow().setAttributes(createLayoutParams(0));
			mRootView.post(this);
		}

		mDialog.show();
	}

	public void dismiss() {
		if (mDialog != null) {
			mDialog.dismiss();
		}

		mDismissTime = System.currentTimeMillis();
	}

	public long getDismissOvertime() {
		return System.currentTimeMillis() - mDismissTime;
	}

	public boolean isShowing() {
		if (mDialog != null) {
			return mDialog.isShowing();
		}

		return false;
	}

	protected void onButtonCancelClicked() {
		dismiss();
	}

	protected void onButtonNowClicked() {
		dismiss();
	}

	@Override
	public void onClick(View v) {
		if (v == mButtonCancel) {
			onButtonCancelClicked();
		} else if (v == mButtonNow) {
			onButtonNowClicked();
		}
	}

	@Override
	public void run() {
		mRootView.removeCallbacks(this);

		if (mDialog != null && mDialog.isShowing()) {
			long time = System.currentTimeMillis();
			if (mCommitTime > time) {
				long delay = mCommitTime - time;
				String text = mContext.getResources().getString(R.string.unpack_delayed, delay / 1000);

				mButtonNow.setText(text);

				if (delay > 1000) {
					delay = 1000;
				}

				mRootView.postDelayed(this, delay);
			} else {
				onButtonNowClicked();
			}
		}
	}
}
