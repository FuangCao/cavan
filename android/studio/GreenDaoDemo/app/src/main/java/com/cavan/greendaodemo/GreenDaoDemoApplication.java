package com.cavan.greendaodemo;

import android.app.Application;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.util.Log;

import com.cavan.greendao.DaoMaster;
import com.cavan.greendao.DaoSession;

import java.io.File;

public class GreenDaoDemoApplication extends Application {

    private DaoSession mDaoSession;

    @Override
    public void onCreate() {
        super.onCreate();

        File dbFile = new File(Environment.getExternalStorageDirectory(), "cavan-greendao.db");
        DaoMaster.DevOpenHelper helper = new DaoMaster.DevOpenHelper(this, dbFile.getAbsolutePath(), null);
        DaoMaster master = new DaoMaster(helper.getWritableDatabase());
        mDaoSession = master.newSession();
    }

    public DaoSession getDaoSession() {
        return mDaoSession;
    }
}
