package com.cavan.touchrecorder;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.AccessibilityServiceInfo;
import android.accessibilityservice.GestureDescription;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.graphics.Path;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import java.util.ArrayList;

public class TouchRecorderService extends AccessibilityService implements View.OnTouchListener, DialogInterface.OnClickListener {

    public static final String TAG = MainActivity.TAG;

    private static final int MSG_SHOW_CONTEXT_MENU = 1;
    private static final int MSG_DISPATCH_GESTURE = 2;
    private static final int MSG_START_PLAY = 3;

    public static TouchRecorderService Instance;

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
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                        dispatchGesture((GestureDescription) msg.obj, null, null);
                    }
                    break;

                case MSG_START_PLAY:
                    StartPlay();
                    break;
            }
        }
    };

    private Thread mPlayThread = new Thread() {

        private boolean mGesturePending;

        public synchronized void setGesturePending(boolean pending) {
            mGesturePending = pending;
            notify();
        }

        public synchronized void waitGestureComplete(long duration) {
            if (mGesturePending) {
                try {
                    wait(duration);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        public long sendMotionEvents(Path path, long duration) {
            if (duration < 1) {
                duration = 1;
            }

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

            if (dispatchGesture(builder.build(), callback, null)) {
                setGesturePending(true);
                return duration;
            }

            return 0;
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

                Path path = null;
                long time = 0;

                for (MotionEventRecod event : mMotionEvents) {
                    if (mPlaying == false) {
                        break;
                    }

                    if (event.mAction == MotionEvent.ACTION_DOWN) {
                        long duration = event.getEventTime() - time;
                        if (duration > 0 && time > 0) {
                            try {
                                wait(duration);
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }

                        path = new Path();
                        path.moveTo(event.mX, event.mY);
                        time = event.getEventTime();
                    } else if (path != null) {
                        path.lineTo(event.mX, event.mY);

                        if (event.mAction == MotionEvent.ACTION_UP) {
                            long duration = sendMotionEvents(path, event.getEventTime() - time);
                            if (duration > 0) {
                                waitGestureComplete(duration);
                            }

                            path = null;
                        }
                    }
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

    public TouchRecorderService() {
    }

    public int getFloatWindowType() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
            return WindowManager.LayoutParams.TYPE_PHONE;
        }

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
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
                view.setText(mMotionEvents.size() + ". "  + MotionEvent.actionToString(event.getAction()) + " (" + event.getX() + ", " + event.getY() + ")");
            }
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

        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.JELLY_BEAN) {
            AccessibilityServiceInfo info = getServiceInfo();
            info.packageNames = getPackageNames();
            info.eventTypes = getEventTypes();
            info.flags = getEventFlags();
            setServiceInfo(info);
            Log.d(TAG, "info = " + getServiceInfo());
        }

        super.onServiceConnected();
    }

    public class MotionEventRecod {

        public float mX;
        public float mY;
        public int mAction;
        public long mEventTime;

        public MotionEventRecod(MotionEvent event) {
            mX = event.getX();
            mY = event.getY();
            mAction = event.getAction();
            mEventTime = event.getEventTime();
        }

        public Path BuildPath()
        {
            Path path = new Path();

            if (mAction == MotionEvent.ACTION_MOVE) {
                Log.d(TAG, "lineTo");
                path.lineTo(mX, mY);
            } else {
                Log.d(TAG, "moveTo");
                path.moveTo(mX, mY);
            }

            return path;
        }

        @NonNull
        @Override
        public String toString() {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
                return MotionEvent.actionToString(mAction) + " (" + mX + ", " + mY + ")";
            }

            return "(" + mX + ", " + mY + ")";
        }

        public long getEventTime() {
            return mEventTime;
        }
    }
}
