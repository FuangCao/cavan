package com.cavan.touchrecorder;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.AccessibilityServiceInfo;
import android.accessibilityservice.GestureDescription;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.graphics.Path;
import android.graphics.PixelFormat;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.accessibility.AccessibilityEvent;
import android.widget.TextView;

import androidx.annotation.NonNull;

import java.util.ArrayList;

public class TouchRecorderService extends AccessibilityService implements View.OnTouchListener, DialogInterface.OnClickListener {

    public static final String TAG = MainActivity.TAG;

    private static final int MSG_SHOW_CONTEXT_MENU = 1;
    private static final int MSG_DISPATCH_GESTURE = 2;
    private static final int MSG_START_PLAY = 3;

    public static TouchRecorderService Instance;

    private boolean mGesturePending;
    private boolean mPlaying;

    private Dialog mAlertDialog;
    private TextView mFloatView;
    private WindowManager mWindorManager;

    private ArrayList<MotionEventRecod> mMotionEvents = new ArrayList<>();

    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(@NonNull Message msg) {
            switch (msg.what) {
                case MSG_SHOW_CONTEXT_MENU:
                    ShowContextMenu();
                    break;

                case MSG_DISPATCH_GESTURE:
                    dispatchGesture((GestureDescription) msg.obj, null, null);
                    break;

                case MSG_START_PLAY:
                    StartPlay();
                    break;
            }
        }
    };

    private Thread mPlayThread = new Thread() {

        public synchronized void setGesturePending(boolean pending) {
            mGesturePending = pending;
            notify();
        }

        public synchronized void waitGestureComplete() {
            while (mGesturePending) {
                try {
                    wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        public synchronized boolean sendMotionEvents(Path path, long duration) {
            if (duration < 1) {
                duration = 1;
            }

            Log.d(TAG, "duration = " + duration);

            GestureDescription.StrokeDescription stroke = new GestureDescription.StrokeDescription(path, 0, duration);
            GestureDescription.Builder builder = new GestureDescription.Builder();
            builder.addStroke(stroke);

            GestureResultCallback callback = new GestureResultCallback() {
                @Override
                public void onCompleted(GestureDescription gestureDescription) {
                    super.onCompleted(gestureDescription);
                    setGesturePending(false);
                }

                @Override
                public void onCancelled(GestureDescription gestureDescription) {
                    super.onCancelled(gestureDescription);
                    setGesturePending(false);
                }
            };

            mGesturePending = true;

            return dispatchGesture(builder.build(), callback, null);
        }

        @Override
        public synchronized void run() {
            while (mPlayThread == this) {
                while (mPlaying == false) {
                    try {
                        wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }

                MotionEventRecod prev = null;
                Path path = null;
                long time = 0;

                for (MotionEventRecod event : mMotionEvents) {
                    if (mPlaying == false) {
                        break;
                    }

                    if (event.getAction() == MotionEvent.ACTION_DOWN) {
                        long duration = event.getEventTime() - time;
                        if (duration > 0 && time > 0) {
                            try {
                                wait(duration);
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }

                        path = new Path();
                        event.moveTo(path);
                        time = event.getEventTime();
                    } else if (path != null) {
                        event.lineTo(prev, path);

                        if (event.getAction() == MotionEvent.ACTION_UP) {
                            try {
                                long duration = event.getEventTime() - time;

                                if (sendMotionEvents(path, duration)) {
                                    waitGestureComplete();
                                }
                            } catch (Exception e) {
                                e.printStackTrace();
                            }

                            path = null;
                        }
                    }

                    prev = event;
                }

                int delay = TouchSettings.PlayDelay(TouchRecorderService.this);
                Log.d(TAG, "delay = " + delay);

                if (delay > 0) {
                    try {
                        wait(delay);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    };

    public int getFloatWindowType() {
        return WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
    }

    public WindowManager.LayoutParams getLayoutParams(boolean not_touch, boolean watch_outside) {
        int flags = 0;

        if (not_touch) {
            flags |= WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
        }

        if (watch_outside) {
            flags |= WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH;
        }

        WindowManager.LayoutParams params = new WindowManager.LayoutParams(
                WindowManager.LayoutParams.MATCH_PARENT,
                WindowManager.LayoutParams.MATCH_PARENT,
                getFloatWindowType(),
                flags,
                PixelFormat.TRANSLUCENT);

        params.gravity = Gravity.CENTER;

        return params;
    }

    public void AddFloatView(boolean not_touch)
    {
        TextView view = mFloatView;
        if (view != null) {
            mWindorManager.updateViewLayout(view, getLayoutParams(not_touch, false));
        } else {
            view = new TextView(this);
            mFloatView = view;

            view.setGravity(Gravity.CENTER);
            view.setOnTouchListener(this);
            mWindorManager.addView(view, getLayoutParams(not_touch, false));
        }
    }

    public void RemoveFloatView()
    {
        TextView view = mFloatView;
        mFloatView = null;

        if (view != null) {
            mWindorManager.removeView(view);
        }
    }

    public boolean StartPlay() {
        Thread thread = mPlayThread;
        if (thread == null) {
            return false;
        }

        synchronized (thread) {
            mGesturePending = false;
            mPlaying = true;

            if (thread.isAlive()) {
                thread.notify();
            } else {
                thread.start();
            }
        }

        return true;
    }

    public boolean StopPlay(boolean exit) {
        Thread thread = mPlayThread;
        if (thread == null) {
            return false;
        }

        synchronized (thread) {
            mGesturePending = false;
            mPlaying = false;

            if (exit) {
                mPlayThread = null;
            }
        }

        return true;
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        switch (which) {
            case 0:
                mMotionEvents.clear();
                AddFloatView(false);
                break;

            case 1:
                AddFloatView(true);
                mHandler.sendEmptyMessageDelayed(MSG_START_PLAY, 1000);
                break;

            case 2:
                RemoveFloatView();
                StopPlay(false);
                break;

            default:
                RemoveFloatView();
                Log.d(TAG, "onClick: " + which);
        }
    }

    public boolean ShowContextMenu() {
        Dialog dialog = mAlertDialog;
        if (dialog != null && dialog.isShowing()) {
            return false;
        }

        StopPlay(false);
        RemoveFloatView();

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setItems(R.array.context_menu, this);
        builder.setCancelable(true);

        mAlertDialog = dialog = builder.create();

        dialog.getWindow().setAttributes(getLayoutParams(false, true));
        dialog.show();

        return true;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Instance = this;
        mWindorManager = (WindowManager) getSystemService(WINDOW_SERVICE);
    }

    @Override
    public void onDestroy() {
        Instance = null;
        StopPlay(true);
        RemoveFloatView();
        super.onDestroy();
    }

    @Override
    protected boolean onKeyEvent(KeyEvent event) {
        Log.d(TAG, "onKeyEvent: " + event);

        if (event.getKeyCode() == KeyEvent.KEYCODE_VOLUME_DOWN && TouchSettings.IsListenVolumeKeyEnabled(this)) {
            if (event.getAction() == KeyEvent.ACTION_DOWN) {
                ShowContextMenu();
            }

            return true;
        }

        return super.onKeyEvent(event);
    }

    @Override
    public void onAccessibilityEvent(AccessibilityEvent accessibilityEvent) {

    }

    @Override
    public void onInterrupt() {

    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        Log.d(TAG, "onTouch: " + event);

        mMotionEvents.add(new MotionEventRecod(event));

        TextView view = mFloatView;
        if (view != null) {
            view.setText(mMotionEvents.size() + ". "  + MotionEvent.actionToString(event.getAction()) + " (" + event.getX() + ", " + event.getY() + ")");
        }

        return false;
    }


    public String[] getPackageNames() {
        return new String[] { getPackageName() };
    }

    public int getEventTypes() {
        return AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED;
    }

    public int getEventFlags() {
        return AccessibilityServiceInfo.DEFAULT | AccessibilityServiceInfo.FLAG_REQUEST_FILTER_KEY_EVENTS;
    }

    @Override
    protected void onServiceConnected() {
        Log.d(TAG, "onServiceConnected");

        AccessibilityServiceInfo info = getServiceInfo();
        info.packageNames = getPackageNames();
        info.eventTypes = getEventTypes();
        info.flags = getEventFlags();
        setServiceInfo(info);
        Log.d(TAG, "info = " + getServiceInfo());

        super.onServiceConnected();
    }

    public class MotionEventRecod {

        private float mX;
        private float mY;
        private int mAction;
        private long mEventTime;

        public MotionEventRecod(MotionEvent event) {
            mX = event.getX();
            mY = event.getY();
            mAction = event.getAction();
            mEventTime = event.getEventTime();
        }

        public float getX() {
            return mX;
        }

        public float getY() {
            return mY;
        }

        public int getAction() {
            return mAction;
        }

        public long getEventTime() {
            return mEventTime;
        }

        @NonNull
        @Override
        public String toString() {
            return MotionEvent.actionToString(mAction) + " (" + mX + ", " + mY + ")";
        }

        public void moveTo(Path path) {
            path.moveTo(mX, mY);
        }

        public void lineTo(MotionEventRecod prev, Path path) {
            path.lineTo(mX, mY);
        }
    }
}
