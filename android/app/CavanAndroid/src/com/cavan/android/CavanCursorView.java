package com.cavan.android;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
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
	private WindowManager.LayoutParams mParams;
	private Paint mPaint;
	private int mType;

	public CavanCursorView(Context context, LayoutParams params) {
		super(context);

		mPaint = new Paint();
		mPaint.setColor(Color.RED);

		mManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);

		mParams = params;
		params.width = WIDTH;
		params.height = WIDTH;
		params.flags |= LayoutParams.FLAG_TRANSLUCENT_STATUS | LayoutParams.FLAG_TRANSLUCENT_NAVIGATION;
		mManager.addView(this, params);
	}

	public void setPosition(int x, int y) {
		if (getVisibility() != View.VISIBLE) {
			return;
		}

		mParams.x = x;
		mParams.y = y;
		mManager.updateViewLayout(this, mParams);
	}

	public void addPosition(int x, int y) {
		setPosition(mParams.x + x, mParams.y + y);
	}

	public int getCursorX() {
		return (int) (mParams.x + RADIUS);
	}

	public int getType() {
		return mType;
	}

	public void setType(int type) {
		mType = type;
	}

	public int getCursorY() {
		return (int) (mParams.y + RADIUS);
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

		int x = getCursorX();
		int y = getCursorY();

		return service.savePosition(mType, x, y);
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

	public void enable(int type) {
		setVisibility(View.VISIBLE);
		mType = type;
		Point point = load();
		setPosition(point.x - RADIUS, point.y - RADIUS);
	}

	public void disable() {
		setVisibility(View.GONE);
	}

	@Override
	protected void onDraw(Canvas canvas) {
		CavanAndroid.dLog("onDraw");

		mPaint.setColor(Color.RED);

		canvas.drawCircle(RADIUS, RADIUS, RADIUS, mPaint);
	}

}
