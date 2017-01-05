package com.cavan.protobufdemo;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

import com.cavan.pb.TestMessage;
import com.google.protobuf.CodedOutputStream;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        TestMessage.Message.Builder builder = TestMessage.Message.newBuilder();
        builder.setId(11);
        builder.setFromUser(11);
        builder.setToUser(22);
        builder.setContent("0123456789");
        TestMessage.Message message = builder.build();

        File txtFile = new File(Environment.getExternalStorageDirectory(), "cavan-protobuf.txt");
        FileOutputStream outStream = null;

        try {
            outStream = new FileOutputStream(txtFile);
            message.writeTo(outStream);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (outStream != null) {
                try {
                    outStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

        FileInputStream inStream = null;

        try {
            inStream = new FileInputStream(txtFile);
            message = TestMessage.Message.parseFrom(inStream);
            Log.d("Cavan", "id = " + message.getId() + ", from_user = " + message.getFromUser() + ", to_user = " + message.getToUser() + ", content = " + message.getContent());
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (inStream != null) {
                try {
                    inStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
