package com.ice.sharedmemory;

import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.util.Log;

import com.ice.ashmem.ISharedMem;

import java.io.IOException;
import java.util.Arrays;

/**
 * Created by ice on 2020/9/9.
 */
public class SharedMemImp extends ISharedMem.Stub {
    private String TAG = SharedMemImp.class.getSimpleName();

    @Override
    public ParcelFileDescriptor getSharedFd() throws RemoteException {
        int[] fd = AshmemWriterHelper.getAshFd();
        Log.i(TAG, "get ash fds: " + Arrays.toString(fd));
        try {
            ParcelFileDescriptor[] parcelFileDescriptors = new ParcelFileDescriptor[3];
            parcelFileDescriptors[0] = ParcelFileDescriptor.fromFd(fd[0]);
            return parcelFileDescriptors[0];
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }
}
