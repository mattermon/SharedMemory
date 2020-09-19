package com.ice.sharedmemory;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

import androidx.annotation.Nullable;

/**
 * Created by ice on 2020/9/9.
 */
public class AshmService extends Service {
    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return new SharedMemImp();
    }
}
