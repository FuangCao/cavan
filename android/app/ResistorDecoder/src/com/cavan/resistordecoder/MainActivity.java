package com.cavan.resistordecoder;

import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.GridView;
import android.widget.Spinner;

public class MainActivity extends ActionBarActivity implements TextWatcher {

	private GridView mGridViewResistor4;
	private GridView mGridViewResistor5;
	private EditText mEditTextResistence;
	private Spinner mSpinnerResistenceUnit;

	private ResistorAdapter mAdapterResistor4;
	private ResistorAdapter mAdapterResistor5;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mGridViewResistor4 = (GridView) findViewById(R.id.gridViewResistor4);
        mAdapterResistor4 = new ResistorAdapter(this, 4);
        mGridViewResistor4.setAdapter(mAdapterResistor4);

        mGridViewResistor5 = (GridView) findViewById(R.id.gridViewResistor5);
        mAdapterResistor5 = new ResistorAdapter(this, 5);
        mGridViewResistor5.setAdapter(mAdapterResistor5);

        mEditTextResistence = (EditText) findViewById(R.id.editTextResistence);
        mEditTextResistence.addTextChangedListener(this);

        mSpinnerResistenceUnit = (Spinner) findViewById(R.id.spinnerResistenceUnit);
        String[] unitList = getResources().getStringArray(R.array.resistence_unit);
        ArrayAdapter<String> unitAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, unitList);
        mSpinnerResistenceUnit.setAdapter(unitAdapter);
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

	@Override
	public void afterTextChanged(Editable s) {
	}

	@Override
	public void beforeTextChanged(CharSequence s, int start, int count, int after) {
	}

	@Override
	public void onTextChanged(CharSequence s, int start, int before, int count) {
		try {
			Float.parseFloat(mEditTextResistence.getText().toString());
		} catch (Exception e) {
		}
	}
}
