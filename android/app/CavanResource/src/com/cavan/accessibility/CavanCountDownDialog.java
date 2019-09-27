package com.cavan.accessibility;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.graphics.PixelFormat;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.widget.Button;

import com.cavan.android.CavanAndroid;
import com.cavan.resource.R;

public class CavanCountDownDialog extends CavanCountDownDialogBase implements OnClickListener {

	protected Context mContext;

	private View mRootView;
	private Button mButtonNow;
	private Button mButtonCancel;

	private Dialog mDialog;

	public CavanCountDownDialog(Context context) {
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
				CavanAndroid.getAlertWindowType(),
				flags,
				PixelFormat.RGBA_8888);

		return params;
	}

	@Override
	public void show(CavanRedPacket packet) {
		super.show(packet);

		if (mDialog == null) {
			AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
			builder.setView(mRootView);
			builder.setCancelable(true);
			mDialog = builder.create();
			mDialog.getWindow().setAttributes(createLayoutParams(0));
		}

		onUpdated(packet.getUnpackDelay(0));
		mDialog.show();
	}

	@Override
	public void dismiss() {
		super.dismiss();

		if (mDialog != null) {
			mDialog.dismiss();
		}
	}

	@Override
	public boolean isShowing() {
		if (mDialog != null) {
			return mDialog.isShowing();
		}

		return false;
	}

	@Override
	public void onUpdated(long delay) {
		String text = mContext.getResources().getString(R.string.unpack_delayed, delay / 1000);
		mButtonNow.setText(text);
	}

	@Override
	public void onClick(View v) {
		if (v == mButtonCancel) {
			onCanceled();
		} else if (v == mButtonNow) {
			onCompleted();
		}
	}
}
