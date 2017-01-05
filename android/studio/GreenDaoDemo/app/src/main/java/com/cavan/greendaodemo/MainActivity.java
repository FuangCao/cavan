package com.cavan.greendaodemo;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

import com.cavan.greendao.DaoMaster;
import com.cavan.greendao.DaoSession;
import com.cavan.greendao.GroupInfo;
import com.cavan.greendao.GroupInfoDao;
import com.cavan.greendao.UserInfo;
import com.cavan.greendao.UserInfoDao;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        GreenDaoDemoApplication app = (GreenDaoDemoApplication) getApplicationContext();
        DaoSession session = app.getDaoSession();
        UserInfoDao userDao = session.getUserInfoDao();

        userDao.insert(new UserInfo(null, "u1", false, 10));
        userDao.insert(new UserInfo(null, "u2", false, 10));

        for (UserInfo info : userDao.loadAll()) {
            Log.d("Cavan", "info = " + info);
        }

        GroupInfoDao groupDao = session.getGroupInfoDao();

        groupDao.insert(new GroupInfo(null, "g1", "1111"));
        groupDao.insert(new GroupInfo(null, "g2", "2222"));

        for (GroupInfo info : groupDao.loadAll()) {
            Log.d("Cavan", "info = " + info);
        }
    }
}
