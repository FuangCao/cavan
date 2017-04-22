package com.cavan.textsearch;

import android.app.Activity;
import android.os.Bundle;
import android.text.Editable;
import android.text.SpannableStringBuilder;
import android.text.TextWatcher;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBusyLock;
import com.cavan.cavanmain.CavanMessageView;
import com.cavan.cavanmain.R;

public class MainActivity extends Activity {

	private Button mButtonPaste;
	private TextView mTextViewContent;
	private EditText mEditTextKeyword;

	private CavanBusyLock mLock = new CavanBusyLock(500) {

		@Override
		public void onBusyLockReleased(Object owner) {
			updateBackgroundColorSpans(mTextViewContent.getText().toString());
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.text_search);

		mButtonPaste = (Button) findViewById(R.id.buttonPaste);
		mButtonPaste.setOnClickListener(new View.OnClickListener() {

			@Override
			public void onClick(View v) {
				updateBackgroundColorSpans(null);
			}
		});

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
	}

	public void updateBackgroundColorSpans(String text) {
		if (text == null) {
			text = CavanAndroid.getClipboardText(this);
			if (text == null) {
				return;
			}
		}

		SpannableStringBuilder builder = new SpannableStringBuilder(text);

		for (String keyword : mEditTextKeyword.getText().toString().split("\\s+[\\n\\|]+\\s+")) {
			keyword = keyword.trim();
			if (keyword.isEmpty()) {
				continue;
			}

			String pattern = keyword.replaceAll("\\*", ".*").replaceAll("\\s+", ".*");
			CavanMessageView.setBackgroundColorSpans(builder, pattern, text);
		}

		mTextViewContent.setText(builder);
	}

	@Override
	protected void onResume() {
		super.onResume();

		Editable editable = mTextViewContent.getEditableText();
		if (editable == null || editable.length() == 0) {
			updateBackgroundColorSpans(null);
		}
	}
}
