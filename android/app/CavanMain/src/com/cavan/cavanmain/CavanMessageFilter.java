package com.cavan.cavanmain;


import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.text.Editable;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ListView;

import com.cavan.android.CavanAndroid;

public class CavanMessageFilter extends DialogFragment implements CompoundButton.OnCheckedChangeListener, View.OnClickListener, DialogInterface.OnClickListener {

    private MessageActivity mMessageActivity;
    private CavanFilter[] mFilters;

    private Button mButtonAdd;
    private Button mButtonDelete;
    private ListView mListViewFilter;
    private EditText mEditTextFilter;
    private CheckBox mCheckBoxSelectAll;


    @SuppressLint("AppCompatCustomView")
    public class CavanFilterView extends CheckBox implements CompoundButton.OnCheckedChangeListener {

        private CavanFilter mFilter;

        public CavanFilterView(Context context, CavanFilter filter) {
            super(context);

            // setTextColor(Color.WHITE);
            setFilter(filter);

            setOnCheckedChangeListener(this);
        }

        public void setFilter(CavanFilter filter) {
            mFilter = filter;

            setText(mFilter.getContent());
            setChecked(mFilter.isEnabled());
        }

        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            mFilter.setEnable(mMessageActivity.getContentResolver(), isChecked);
        }
    }

    private BaseAdapter mFilterAdapter = new BaseAdapter() {

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            CavanFilterView view = (CavanFilterView) convertView;
            CavanFilter filter = mFilters[position];
            if (view == null) {
                view = new CavanFilterView(mMessageActivity, filter);
            } else {
                view.setFilter(filter);
            }

            return view;
        }

        @Override
        public long getItemId(int position) {
            return 0;
        }

        @Override
        public Object getItem(int position) {
            return null;
        }

        @Override
        public int getCount() {
            if (mFilters != null) {
                return mFilters.length;
            }

            return 0;
        }

        @Override
        synchronized public void notifyDataSetChanged() {
            mFilters = CavanFilter.queryFilter(mMessageActivity.getContentResolver());
            super.notifyDataSetChanged();
        }
    };

    public void show(FragmentManager manager) {
        super.show(manager, CavanAndroid.TAG);
    }

    public void addFilter(String text) {
        if (!text.isEmpty()) {
            CavanFilter filter = new CavanFilter(text, true);
            filter.update(mMessageActivity.getContentResolver());
        }
    }

    public MessageActivity getMessageActivity() {
        return mMessageActivity;
    }

    public void setMessageActivity(MessageActivity activity) {
        mMessageActivity = activity;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        View view = mMessageActivity.getLayoutInflater().inflate(R.layout.message_filter, null);

        mCheckBoxSelectAll = (CheckBox) view.findViewById(R.id.checkBoxSelectAll);
        // mCheckBoxSelectAll.setTextColor(Color.WHITE);
        mCheckBoxSelectAll.setOnCheckedChangeListener(this);

        mButtonAdd = (Button) view.findViewById(R.id.buttonAdd);
        mButtonAdd.setOnClickListener(this);

        mButtonDelete = (Button) view.findViewById(R.id.buttonDelete);
        mButtonDelete.setOnClickListener(this);

        mEditTextFilter = (EditText) view.findViewById(R.id.editTextFilter);

        mListViewFilter = (ListView) view.findViewById(R.id.listViewFilter);
        mListViewFilter.setAdapter(mFilterAdapter);
        mFilterAdapter.notifyDataSetChanged();

        AlertDialog.Builder builder = new AlertDialog.Builder(mMessageActivity);

        builder.setView(view);
        builder.setCancelable(false);
        builder.setPositiveButton(R.string.filter, this);
        builder.setNegativeButton(R.string.filter_none, this);

        return builder.create();
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:
                mMessageActivity.setFilterEnable(true);
                break;

            case DialogInterface.BUTTON_NEGATIVE:
                mMessageActivity.setFilterEnable(false);
                break;
        }
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        addFilter(mEditTextFilter.getText().toString());
        mMessageActivity.updateFilter();

        super.onDismiss(dialog);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.buttonAdd:
                Editable editable = mEditTextFilter.getText();
                addFilter(editable.toString());
                mFilterAdapter.notifyDataSetChanged();
                editable.clear();
                break;

            case R.id.buttonDelete:
                CavanFilter.deleteEnabled(mMessageActivity.getContentResolver());
                mFilterAdapter.notifyDataSetChanged();
                break;
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (mFilters != null) {
            for (int i = mFilters.length - 1; i >= 0; i--) {
                mFilters[i].setEnable(mMessageActivity.getContentResolver(), isChecked);
            }

            mFilterAdapter.notifyDataSetChanged();
        }
    }
}