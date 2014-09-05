package com.cavan.huamobile;

import com.cavan.huahardwareinfo.R;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.app.Activity;
import android.app.ActivityManager;
import android.util.Log;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;

public class HuaAdbRootActivity extends Activity implements OnCheckedChangeListener {
	private final String TAG = "Cavan";

    private CheckBox mCheckBoxAdbRoot;
    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            Settings.Secure.putInt(getContentResolver(), Settings.Secure.ADB_ENABLED, 1);
            mCheckBoxAdbRoot.setEnabled(true);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hua_adb_root);

        mCheckBoxAdbRoot = (CheckBox) findViewById(R.id.checkBoxAdbRoot);
        mCheckBoxAdbRoot.setChecked(SystemProperties.getInt("persist.service.adb.root", 0) > 0);

        if (ActivityManager.isUserAMonkey()) {
    		Log.d(TAG, getClass() + " in monkey test mode");
    	} else {
    		mCheckBoxAdbRoot.setOnCheckedChangeListener(this);
    	}
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        mCheckBoxAdbRoot.setEnabled(false);
        SystemProperties.set("persist.service.adb.root", isChecked ? "1" : "0");
        Settings.Secure.putInt(getContentResolver(), Settings.Secure.ADB_ENABLED, 0);
        mHandler.sendEmptyMessageDelayed(0, 1500);
    }
}
