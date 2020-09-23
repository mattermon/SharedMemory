package com.ice.ashmemreader;

import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.util.Log;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;

import com.ice.ashmem.ISharedMem;

public class MainActivity extends AppCompatActivity {

    private String TAG = "Reader";

    ServiceConnection conn = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            ISharedMem sharedMem = ISharedMem.Stub.asInterface(service);
            try {
                ParcelFileDescriptor descriptor = sharedMem.getSharedFd();
                int fd = descriptor.getFd();
                Log.d(TAG, "get fd: " + fd);
                AshReaderHelper.init(descriptor.getFd());
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {

        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void init(View view) {
//        Intent intent = new Intent();
//        intent.setComponent(new ComponentName("com.ice.sharedmemory", "com.ice.sharedmemory.AshmService"));
//        bindService(intent, conn, Service.BIND_AUTO_CREATE);

        AshReaderHelper.init(0);
    }

    public void read(View view) {
        AshReaderHelper.read();
    }

    public void getFdBySocket(View view) {
        AshReaderHelper.getFdBySocket();
    }








}
