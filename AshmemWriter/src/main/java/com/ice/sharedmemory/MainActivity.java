package com.ice.sharedmemory;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.view.View;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void init(View view) {
        AshmemWriterHelper.initAshmem();
    }

    int i = 0;

    public void write(View view) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                while (true) {
                    AshmemWriterHelper.write(i, i + " " + i + "" + i + " " + i + " " + i);
                    i++;
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        }).start();
    }
}