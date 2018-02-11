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

import com.cavan.accessibility.CavanAccessibilityPackage;
import com.cavan.accessibility.CavanRedPacket;

public class CountDownDialog implements OnClickListener {

	protected Context mContext;
	protected CavanRedPacket mPacket;

	private View mRootView;
	private Button mButtonNow;
	private Button mButtonCancel;

	private Dialog mDialog;

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

	public void show(CavanRedPacket packet, long delay) {
		mPacket = packet;

		if (mDialog == null) {
			AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
			builder.setView(mRootView);
			builder.setCancelable(true);
			mDialog = builder.create();
			mDialog.getWindow().setAttributes(createLayoutParams(0));
		}

		String text = mContext.getResources().getString(R.string.unpack_delayed, delay / 1000);
		mButtonNow.setText(text);
		mDialog.show();
	}

	public void dismiss() {
		if (mDialog != null) {
			mDialog.dismiss();
		}
	}

	public boolean isShowing() {
		if (mDialog != null) {
			return mDialog.isShowing();
		}

		return false;
	}

	protected void onButtonCancelClicked() {
		CavanAccessibilityPackage pkg = mPacket.getPackage();
		pkg.setForceUnpackEnable(false);
		mPacket.setGotoIdle(false);
		pkg.removePackets();
		dismiss();
	}

	protected void onButtonNowClicked() {
		mPacket.setGotoIdle(false);
		mPacket.setUnpackTime(0);
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
}
