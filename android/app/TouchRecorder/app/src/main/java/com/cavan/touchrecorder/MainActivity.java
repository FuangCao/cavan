package com.cavan.touchrecorder;

import android.Manifest;
import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {

    public static final String TAG = "TouchRecorder";

    public static String[] Permissions = new String[] {
            Manifest.permission.SYSTEM_ALERT_WINDOW
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getFragmentManager().beginTransaction().replace(android.R.id.content, TouchSettings.getInstance()).commit();
    }
}
