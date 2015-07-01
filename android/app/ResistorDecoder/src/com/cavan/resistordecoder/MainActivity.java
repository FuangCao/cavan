package com.cavan.resistordecoder;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.ActionBarActivity;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.GridView;
import android.widget.Spinner;

import com.cavan.resistordecoder.ResistorAdapter.OnResistenceChangedListener;

public class MainActivity extends ActionBarActivity implements TextWatcher, OnItemSelectedListener, OnResistenceChangedListener {

	private GridView mGridViewResistor4;
	private GridView mGridViewResistor5;
	private EditText mEditTextResistence;
	private Spinner mSpinnerResistenceUnit;
	private Spinner mSpinnerResistenceMistake;

	private ResistorAdapter mAdapterResistor4;
	private ResistorAdapter mAdapterResistor5;

	private static boolean mBusy;

	private static Handler mHandlerBusy = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			mBusy = false;
			super.handleMessage(msg);
		}
	};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mGridViewResistor4 = (GridView) findViewById(R.id.gridViewResistor4);
        mAdapterResistor4 = new ResistorAdapter(this, 4);
        mGridViewResistor4.setAdapter(mAdapterResistor4);
        mAdapterResistor4.setOnResistenceChangedListener(this);

        mGridViewResistor5 = (GridView) findViewById(R.id.gridViewResistor5);
        mAdapterResistor5 = new ResistorAdapter(this, 5);
        mGridViewResistor5.setAdapter(mAdapterResistor5);
        mAdapterResistor5.setOnResistenceChangedListener(this);

        mEditTextResistence = (EditText) findViewById(R.id.editTextResistence);
        mEditTextResistence.addTextChangedListener(this);

        mSpinnerResistenceUnit = (Spinner) findViewById(R.id.spinnerResistenceUnit);
        String[] unitList = getResources().getStringArray(R.array.resistence_unit);
        ArrayAdapter<String> unitAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, unitList);
        mSpinnerResistenceUnit.setAdapter(unitAdapter);
        mSpinnerResistenceUnit.setOnItemSelectedListener(this);

        mSpinnerResistenceMistake = (Spinner) findViewById(R.id.spinnerResistenceMitake);
        String[] mistakeList = getResources().getStringArray(R.array.resistence_mistake);
        ArrayAdapter<String> mistakeAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, mistakeList);
        mSpinnerResistenceMistake.setAdapter(mistakeAdapter);
        mSpinnerResistenceMistake.setOnItemSelectedListener(this);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private void showResistence() {
		try {
			double unit = Math.pow(1000, mSpinnerResistenceUnit.getSelectedItemPosition());
			double resistence = Double.parseDouble(mEditTextResistence.getText().toString()) * unit;

			mAdapterResistor4.setResistence(resistence);
			mAdapterResistor5.setResistence(resistence);
		} catch (Exception e) {
		}
    }

	private boolean setBusy(boolean busy) {
		if (mBusy == busy) {
			return false;
		}

		if (busy) {
			mBusy = true;
		} else {
			mHandlerBusy.sendEmptyMessageDelayed(0, 100);
		}

		return true;
	}

	@Override
	public void afterTextChanged(Editable s) {
	}

	@Override
	public void beforeTextChanged(CharSequence s, int start, int count, int after) {
	}

	@Override
	public void onTextChanged(CharSequence s, int start, int before, int count) {
		if (setBusy(true)) {
			showResistence();
			setBusy(false);
		}
	}

	@Override
	public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
		if (setBusy(true)) {
			showResistence();
			setBusy(false);
		}
	}

	@Override
	public void onNothingSelected(AdapterView<?> arg0) {
	}

	@Override
	public void onResistenceChanged(ResistorAdapter adapter) {
		if (!setBusy(true)) {
			return;
		}

		double resistence = adapter.getResistence();
		if (adapter == mAdapterResistor4) {
			mAdapterResistor5.setResistence(resistence);
		} else {
			mAdapterResistor4.setResistence(resistence);
		}

		if (resistence > 1000000) {
			resistence /= 1000000;
			mSpinnerResistenceUnit.setSelection(2);
		} else if (resistence > 1000) {
			resistence /= 1000;
			mSpinnerResistenceUnit.setSelection(1);
		} else {
			mSpinnerResistenceUnit.setSelection(0);
		}

		mEditTextResistence.setText(Double.toString(resistence));

		setBusy(false);
	}
}
