package com.cavan.textsearch;

import android.app.Activity;
import android.graphics.Rect;
import android.os.Bundle;
import android.text.Editable;
import android.text.Layout;
import android.text.SpannableStringBuilder;
import android.text.TextWatcher;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBusyLock;
import com.cavan.cavanmain.CavanMessageView;
import com.cavan.cavanmain.R;

public class MainActivity extends Activity {

	private TextView mTextViewContent;
	private EditText mEditTextKeyword;

	private int mIndex;
	private SpannableStringBuilder mBuilder;

	private CavanBusyLock mLock = new CavanBusyLock(500) {

		@Override
		public void onBusyLockReleased(Object owner) {
			mIndex = -1;

			if (mBuilder == null) {
				return;
			}

			mBuilder.clearSpans();

			for (String keyword : mEditTextKeyword.getText().toString().split("\\s+[\\n\\|]+\\s+")) {
				keyword = keyword.trim();
				if (keyword.isEmpty()) {
					continue;
				}

				String pattern = keyword.replaceAll("\\*", ".*?").replaceAll("\\s+", ".*?");

				int index = CavanMessageView.setBackgroundColorSpans(mBuilder, pattern);
				if (mIndex <= 0) {
					mIndex = index;
				}
			}

			mTextViewContent.post(mRunnableUpdate);
		}

		@Override
		public boolean doPostRunnable(Runnable runnable, long delayMillis) {
			return CavanAndroid.postRunnableThreaded(runnable, delayMillis);
		}

		@Override
		public void doRemoveRunnable(Runnable runnable) {
			CavanAndroid.removeRunnableThreaded(runnable);
		}
	};

	private Runnable mRunnableUpdate = new Runnable() {

		@Override
		public void run() {
			Layout layout = mTextViewContent.getLayout();
			mTextViewContent.setText(mBuilder);

			if (layout != null && mBuilder != null) {
				int line = layout.getLineForOffset(mIndex);
				Rect bounds = new Rect();
				layout.getLineBounds(line, bounds);
				mTextViewContent.scrollTo(bounds.left, bounds.top);
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.text_search);

		mEditTextKeyword = (EditText) findViewById(R.id.editTextKeyword);
		mEditTextKeyword.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void afterTextChanged(Editable s) {
				mLock.acquire(mEditTextKeyword);
			}
		});

		mTextViewContent = (TextView) findViewById(R.id.textViewContent);
		mTextViewContent.setOnClickListener(new View.OnClickListener() {

			@Override
			public void onClick(View v) {
				CavanAndroid.setSoftInputEnable(getApplicationContext(), mEditTextKeyword, false);
			}
		});

		mTextViewContent.setMovementMethod(ScrollingMovementMethod.getInstance());
	}

	@Override
	protected void onResume() {
		super.onResume();

		if (mBuilder == null) {
			String text = CavanAndroid.getClipboardText(this);
			if (text != null && text.length() > 0) {
				mBuilder = new SpannableStringBuilder(text);
				mTextViewContent.setText(mBuilder);
			}
		}
	}
}
