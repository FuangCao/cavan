package com.cavan.resistordecoder;

import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.GridView;


public class MainActivity extends ActionBarActivity {

	private GridView mGridViewResistor4;
	private GridView mGridViewResistor5;
	private ResistorAdapter mAdapterResistor4;
	private ResistorAdapter mAdapterResistor5;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mGridViewResistor4 = (GridView) findViewById(R.id.gridViewResistor4);
        mAdapterResistor4 = new ResistorAdapter(getApplicationContext(), 4);
        mGridViewResistor4.setAdapter(mAdapterResistor4);

        mGridViewResistor5 = (GridView) findViewById(R.id.gridViewResistor5);
        mAdapterResistor5 = new ResistorAdapter(getApplicationContext(), 5);
        mGridViewResistor5.setAdapter(mAdapterResistor5);
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
}
