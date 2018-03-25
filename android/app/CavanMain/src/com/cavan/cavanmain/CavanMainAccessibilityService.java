package com.cavan.cavanmain;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.view.WindowManager;
import android.view.accessibility.AccessibilityEvent;

import com.cavan.accessibility.CavanAccessibilityAlipay;
import com.cavan.accessibility.CavanAccessibilityPackage;
import com.cavan.accessibility.CavanAccessibilityService;
import com.cavan.accessibility.CavanKeyguardActivity;
import com.cavan.android.CavanAndroid;

public class CavanMainAccessibilityService extends CavanAccessibilityService {

	public static CavanMainAccessibilityService instance;

	public static boolean checkAndOpenSettingsActivity(Context context) {
        if (instance != null) {
            return true;
        }

        PermissionSettingsActivity.startAccessibilitySettingsActivity(context);

        return false;
    }

	public CavanMainAccessibilityService() {
		addPackage(new CavanMainAccessibilityMM(this));
		addPackage(new CavanMainAccessibilityQQ(this));
		addPackage(new CavanAccessibilityAlipay(this));
	}

	@Override
	public int getEventTypes() {
		return AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED | AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED;
	}

	@Override
	public Class<?> getBroadcastReceiverClass() {
		return CavanBroadcastReceiver.class;
	}

	@Override
	protected String getInputMethodName() {
		return getResources().getString(R.string.cavan_input_method);
	}

	@Override
	public boolean onShowLoginDialog(CavanAccessibilityPackage pkg) {
		final CavanUserInfo[] users = CavanUserInfo.query(getContentResolver(), pkg.getName(), null);
		if (users == null) {
			return false;
		}

		if (users.length < 2) {
			if (users.length > 0) {
				CavanUserInfo info = users[0];
				return login(info.getAccount(), info.getPassword());
			}

			return false;
		}

		String[] items = new String[users.length];
		for (int i = 0; i < items.length; i++) {
			items[i] = users[i].getAccount();
		}

		AlertDialog.Builder builder = new AlertDialog.Builder(this);

		builder.setSingleChoiceItems(items, 0, new OnClickListener() {

			@Override
			public void onClick(DialogInterface dialog, int which) {
				dialog.dismiss();
				CavanUserInfo info = users[which];
				login(info.getAccount(), info.getPassword());
			}
		});

		builder.setCancelable(true);
		AlertDialog dialog = builder.create();
		dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
		dialog.show();

		return true;
	}

	@Override
	public String getPassword(CavanAccessibilityPackage pkg, String username) {
		CavanUserInfo[] users = CavanUserInfo.query(getContentResolver(), pkg.getName(), username);
		if (users != null && users.length == 1) {
			return users[0].getPassword();
		}

		return null;
	}

	@Override
	public void onCreate() {
		super.onCreate();
		instance = this;
	}

	@Override
	public void onDestroy() {
		instance = null;
		super.onDestroy();
	}

	@Override
	protected void onScreenOff() {
		if (CavanMessageActivity.isDisableKeyguardEnabled(this)) {
			CavanKeyguardActivity.show(this);
		} else {
			CavanAndroid.startLauncher(this);
		}
	}
}
