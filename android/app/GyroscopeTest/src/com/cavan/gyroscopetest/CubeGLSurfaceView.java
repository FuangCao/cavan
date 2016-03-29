package com.cavan.gyroscopetest;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.opengl.GLUtils;
import android.util.AttributeSet;

@SuppressLint("NewApi")
public class CubeGLSurfaceView extends GLSurfaceView implements Renderer {

	private static final int one = 0x10000;

	private float mRotateX;
	private float mRotateY;
	private float mRotateZ;

	private Context mContext;
	private int[] mTextures;
	private Bitmap[] mBitmaps;
	private final int mBitmapResources[] = {
			R.drawable.frog1,
			R.drawable.frog2,
			R.drawable.frog3,
			R.drawable.frog4,
			R.drawable.frog5,
			R.drawable.frog6
	};

	IntBuffer vertices = buildIntBuffer(new int[] {
			-one, -one, one,		one, -one, one,		one, one, one,		-one, one, one,
			-one, one, -one,		one, one, -one,		one, -one, -one,	-one, -one, -one,
			-one, one, one,			one, one, one,		one, one, -one,		-one, one, -one,
			-one, -one, -one,		one, -one, -one,	one, -one, one,		-one, -one, one,
			one, -one, one,			one, -one, -one,	one, one, -one,		one, one, one,
			-one, -one, -one,		-one, -one, one,	-one, one, one,		-one, one, -one
	});

	IntBuffer texCoords = buildIntBuffer(new int[] {
			one, one,				0, one,				0, 0,				one, 0,
			one, one,				0, one,				0, 0,				one, 0,
			one, one,				0, one,				0, 0,				one, 0,
			one, one,				0, one,				0, 0,				one, 0,
			one, one,				0, one,				0, 0,				one, 0,
			one, one,				0, one,				0, 0,				one, 0,
	});

	ByteBuffer[] mIndices = {
			ByteBuffer.wrap(new byte[] { 0, 1, 3, 2}),
			ByteBuffer.wrap(new byte[] { 4, 5, 7, 6 }),
			ByteBuffer.wrap(new byte[] { 8, 9, 11, 10 }),
			ByteBuffer.wrap(new byte[] { 12, 13, 15, 14 }),
			ByteBuffer.wrap(new byte[] { 16, 17, 19, 18 }),
			ByteBuffer.wrap(new byte[] { 20, 21, 23, 22 })
	};

	public static IntBuffer buildIntBuffer(int[] data) {
		ByteBuffer byteBuffer = ByteBuffer.allocateDirect(data.length * 4);
		byteBuffer.order(ByteOrder.nativeOrder());

		IntBuffer intBuffer = byteBuffer.asIntBuffer();
		intBuffer.put(data);
		intBuffer.position(0);

		return intBuffer;
	}

	public CubeGLSurfaceView(Context context, AttributeSet attrs) {
		super(context, attrs);
		mContext = context;
		init();
	}

	public CubeGLSurfaceView(Context context) {
		super(context);
		mContext = context;
		init();
	}

	public void init() {
		Resources resources = mContext.getResources();
		mBitmaps = new Bitmap[mBitmapResources.length];
		for (int i = 0; i < mBitmaps.length; i++) {
			mBitmaps[i] = BitmapFactory.decodeResource(resources, mBitmapResources[i]);
		}

		setRenderer(this);
	}

	@Override
	public void onDrawFrame(GL10 gl) {
		// 清除屏幕和深度缓存
		gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);

		gl.glMatrixMode(GL10.GL_MODELVIEW);
		// 重置当前的模型观察矩阵
		gl.glLoadIdentity();

		gl.glTranslatef(0.0f, 0.0f, -3.0f);

		// 设置旋转
		gl.glRotatef(mRotateX, 1.0f, 0.0f, 0.0f);
		gl.glRotatef(mRotateY, 0.0f, 1.0f, 0.0f);
		gl.glRotatef(mRotateZ, 0.0f, 0.0f, 1.0f);

		// gl.glNormalPointer(GL10.GL_FIXED, 0, normals);
		gl.glVertexPointer(3, GL10.GL_FIXED, 0, vertices);
		gl.glTexCoordPointer(2, GL10.GL_FIXED, 0, texCoords);

		// gl.glEnableClientState(GL10.GL_NORMAL_ARRAY);
		gl.glEnableClientState(GL10.GL_VERTEX_ARRAY);
		gl.glEnableClientState(GL10.GL_TEXTURE_COORD_ARRAY);

		// 绘制四边形
		for (int i = 0; i < mTextures.length; i++) {
			gl.glBindTexture(GL10.GL_TEXTURE_2D, mTextures[i]);
			gl.glDrawElements(GL10.GL_TRIANGLE_STRIP, 4, GL10.GL_UNSIGNED_BYTE, mIndices[i]);
		}

		gl.glDisableClientState(GL10.GL_TEXTURE_COORD_ARRAY);
		gl.glDisableClientState(GL10.GL_VERTEX_ARRAY);
	}

	@Override
	public void onSurfaceChanged(GL10 gl, int width, int height) {
		float ratio = (float) width / height;
		// 设置OpenGL场景的大小
		gl.glViewport(0, 0, width, height);
		// 设置投影矩阵
		gl.glMatrixMode(GL10.GL_PROJECTION);
		// 重置投影矩阵
		gl.glLoadIdentity();
		// 设置视口的大小
		gl.glFrustumf(-ratio, ratio, -1, 1, 1, 10);
		// 选择模型观察矩阵
		gl.glMatrixMode(GL10.GL_MODELVIEW);
		// 重置模型观察矩阵
		gl.glLoadIdentity();
	}

	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		// 告诉系统对透视进行修正
		gl.glHint(GL10.GL_PERSPECTIVE_CORRECTION_HINT, GL10.GL_FASTEST);
		// 黑色背景
		gl.glClearColor(0, 0, 0, 0);

		gl.glEnable(GL10.GL_CULL_FACE);
		// 启用阴影平滑
		gl.glShadeModel(GL10.GL_SMOOTH);
		// 启用深度测试
		gl.glEnable(GL10.GL_DEPTH_TEST);

		IntBuffer textureBuffer = IntBuffer.allocate(mBitmaps.length);
		gl.glGenTextures(mBitmaps.length, textureBuffer);
		mTextures = textureBuffer.array();

		for (int i = 0; i < mBitmaps.length; i++) {
			gl.glBindTexture(GL10.GL_TEXTURE_2D, mTextures[i]);
			GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, mBitmaps[i], 0);
			gl.glTexParameterx(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_NEAREST);
			gl.glTexParameterx(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_NEAREST);
		}

		gl.glEnable(GL10.GL_TEXTURE_2D);
	}

	public void rotateAdd(double x, double y, double z) {
		mRotateX = (float) ((mRotateX + x) % 360.0);
		mRotateY = (float) ((mRotateY + y) % 360.0);
		mRotateZ = (float) ((mRotateZ + z) % 360.0);
	}
}
