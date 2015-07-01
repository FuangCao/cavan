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

public class MainActivity extends ActionBarActivity implements OnItemSelectedListener, OnResistenceChangedListener {

	private GridView mGridViewResistor4;
	private ResistorAdapter mAdapterResistor4;

	private GridView mGridViewResistor5;
	private ResistorAdapter mAdapterResistor5;

	private GridView mGridViewResistor6;
	private ResistorAdapter mAdapterResistor6;

	private EditText mEditTextResistence1;
	private EditText mEditTextResistence2;
	private Spinner mSpinnerResistenceUnit;
	private Spinner mSpinnerResistenceMistake;
	private Spinner mSpinnerTempCofficient;

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

        mGridViewResistor6 = (GridView) findViewById(R.id.gridViewResistor6);
        mAdapterResistor6 = new ResistorAdapter(this, 6);
        mGridViewResistor6.setAdapter(mAdapterResistor6);
        mAdapterResistor6.setOnResistenceChangedListener(this);

        mEditTextResistence1 = (EditText) findViewById(R.id.editTextResistence1);
        mEditTextResistence1.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {
				showResistence(mEditTextResistence1);
			}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {
			}

			@Override
			public void afterTextChanged(Editable s) {
			}
		});

        mEditTextResistence2 = (EditText) findViewById(R.id.editTextResistence2);
        mEditTextResistence2.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {
				showResistence(mEditTextResistence2);
			}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {
			}

			@Override
			public void afterTextChanged(Editable s) {
			}
		});

        mSpinnerResistenceUnit = (Spinner) findViewById(R.id.spinnerResistenceUnit);
        String[] unitList = getResources().getStringArray(R.array.resistence_unit);
        ArrayAdapter<String> unitAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, unitList);
        mSpinnerResistenceUnit.setAdapter(unitAdapter);
        mSpinnerResistenceUnit.setOnItemSelectedListener(this);

        mSpinnerResistenceMistake = (Spinner) findViewById(R.id.spinnerResistenceMistake);
        String[] mistakeList = getResources().getStringArray(R.array.resistence_mistake);
        ArrayAdapter<String> mistakeAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, mistakeList);
        mSpinnerResistenceMistake.setAdapter(mistakeAdapter);
        mSpinnerResistenceMistake.setOnItemSelectedListener(this);

        mSpinnerTempCofficient = (Spinner) findViewById(R.id.spinnerTempCofficient);
        String[] tempCofficient = getResources().getStringArray(R.array.temperature_coefficient);
        ArrayAdapter<String> tempCofficientAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, tempCofficient);
        mSpinnerTempCofficient.setAdapter(tempCofficientAdapter);
        mSpinnerTempCofficient.setOnItemSelectedListener(this);
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

    private void showResistence(Object object) {
		if (!setBusy(true)) {
			return;
		}

		try {
			double resistence;
			int mistake;
			int tempCofficient;

			if (object instanceof ResistorAdapter) {
				ResistorAdapter adapter = (ResistorAdapter) object;
				resistence = adapter.getResistence();
				mistake = adapter.getMistake();
				tempCofficient = adapter.getTempCofficient();

				mSpinnerResistenceMistake.setSelection(adapter.getMistake());
				mSpinnerTempCofficient.setSelection(adapter.getTempCofficient());
			} else {
				if (object == mEditTextResistence1) {
					double unit = Math.pow(1000, mSpinnerResistenceUnit.getSelectedItemPosition());

					resistence = Double.parseDouble(mEditTextResistence1.getText().toString()) * unit;
					mistake = mSpinnerResistenceMistake.getSelectedItemPosition();
				} else {
					String text = mEditTextResistence2.getText().toString();

					mistake = text.length() < 4 ? 6 : 0;

					int index = text.indexOf('R');
					if (index < 0) {
						long value = Long.parseLong(text);
						resistence = (value / 10) * Math.pow(10, value % 10);
					} else {
						if (index == 0) {
							text = "0." + text.substring(1);
						} else {
							text = text.replace('R', '.');
						}

						resistence = Double.parseDouble(text);
					}

					mSpinnerResistenceMistake.setSelection(mistake);
				}

				tempCofficient = mSpinnerTempCofficient.getSelectedItemPosition();
			}

			if (object != mAdapterResistor4) {
				mAdapterResistor4.setResistence(resistence, mistake, tempCofficient);
			}

			if (object != mAdapterResistor5) {
				mAdapterResistor5.setResistence(resistence, mistake, tempCofficient);
			}

			if (object != mAdapterResistor6) {
				mAdapterResistor6.setResistence(resistence, mistake, tempCofficient);
			}

			if (object != mEditTextResistence1) {
				double value = resistence;

				if (value >= 1000000) {
					value /= 1000000;
					mSpinnerResistenceUnit.setSelection(2);
				} else if (value >= 1000) {
					value /= 1000;
					mSpinnerResistenceUnit.setSelection(1);
				} else {
					mSpinnerResistenceUnit.setSelection(0);
				}

				mEditTextResistence1.setText(Double.toString(value));
			}

			if (object != mEditTextResistence2) {
				int pow;
				double value = resistence;

				if (value < 1) {
					mEditTextResistence2.setText(String.format("R%03d", (int) (value * 1000)));
				} else if (value < 10) {
					mEditTextResistence2.setText(String.format("%dR%d", ((int) value) % 10, ((int) (value * 10)) % 10));
				} else {
					for (pow = 0, value = resistence; value > 99 && pow < 9; value /= 10, pow++);

					mEditTextResistence2.setText(String.format("%03d", ((int) value) * 10 + pow));
				}
			}
		} catch (Exception e) {
		} finally {
			setBusy(false);
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
	public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
		showResistence(mEditTextResistence1);
	}

	@Override
	public void onNothingSelected(AdapterView<?> arg0) {
	}

	@Override
	public void onResistenceChanged(ResistorAdapter adapter) {
		showResistence(adapter);
	}
}
