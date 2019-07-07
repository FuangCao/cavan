package com.cavan.android;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import com.cavan.accessibility.CavanAccessibilityService;

public class CavanCursorView extends View {

	public static final int WIDTH = 30;
	public static final int RADIUS = WIDTH / 2;

	public static int TYPE_LOGIN = 0;
	public static int TYPE_LOGOUT = 1;
	public static int TYPE_COUNT = 2;

	private WindowManager mManager;
	private Paint mPaint;
	private int mType;
	private int mViewX;
	private int mViewY;
	private int mRawX;
	private int mRawY;

	public CavanCursorView(Context context, LayoutParams params) {
		super(context);

		mPaint = new Paint();
		mPaint.setColor(Color.RED);

		mManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);

		params = new LayoutParams(
				WindowManager.LayoutParams.MATCH_PARENT,
				WindowManager.LayoutParams.MATCH_PARENT,
				LayoutParams.TYPE_PHONE, // LayoutParams.TYPE_TOAST,
				LayoutParams.FLAG_NOT_FOCUSABLE | LayoutParams.FLAG_TRANSLUCENT_STATUS | LayoutParams.FLAG_TRANSLUCENT_NAVIGATION,
				PixelFormat.RGBA_8888);

		mManager.addView(this, params);
	}

	public int getType() {
		return mType;
	}

	public void setType(int type) {
		mType = type;
	}

	public Point load() {
		CavanAccessibilityService service = CavanAccessibilityService.instance;
		if (service != null) {
			Point point = service.readPosition(mType);
			if (point != null) {
				return point;
			}
		}

		Point point = new Point();

		mManager.getDefaultDisplay().getRealSize(point);
		point.set(point.x / 2, point.y / 2);

		return point;
	}

	public boolean save() {
		if (getVisibility() != View.VISIBLE) {
			return false;
		}

		CavanAccessibilityService service = CavanAccessibilityService.instance;
		if (service == null) {
			return false;
		}

		return service.savePosition(mType, mRawX, mRawY);
	}

	public boolean remove() {
		if (getVisibility() != View.VISIBLE) {
			return false;
		}

		CavanAccessibilityService service = CavanAccessibilityService.instance;
		if (service == null) {
			return false;
		}

		return service.removePosition(mType);
	}

	public boolean tap() {
		if (getVisibility() != View.VISIBLE) {
			return false;
		}

		CavanAccessibilityService service = CavanAccessibilityService.instance;
		if (service == null) {
			return false;
		}

		return service.tapPosition(new Point(mRawX, mRawY));
	}

	public void enable(int type) {
		setVisibility(View.VISIBLE);
		mType = type;

		Point point = load();
		setPosition(point.x, point.y);
	}

	public void disable() {
		setVisibility(View.GONE);
	}

	public boolean setPosition(int x, int y) {
		if (getVisibility() != View.VISIBLE) {
			return false;
		}

		mViewX = mRawX = x;
		mViewY = mRawY = y;
		postInvalidate();

		return true;
	}

	public boolean addPosition(int x, int y) {
		if (getVisibility() != View.VISIBLE) {
			return false;
		}

		mViewX += x;
		mViewY += y;
		mRawX += x;
		mRawY += y;
		postInvalidate();

		return true;
	}

	@SuppressLint("ClickableViewAccessibility")
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		mRawX = (int) event.getRawX();
		mRawY = (int) event.getRawY();
		mViewX = (int) event.getX();
		mViewY = (int) event.getY();
		postInvalidate();
		return true;
	}

	@Override
	protected void onDraw(Canvas canvas) {
		String text = String.format("(%d, %d) (%d, %d)", mViewX, mViewY, mRawX, mRawY);
		canvas.drawText(text, 0, 0, mPaint);
		canvas.drawCircle(mViewX, mViewY, RADIUS, mPaint);
	}
}
