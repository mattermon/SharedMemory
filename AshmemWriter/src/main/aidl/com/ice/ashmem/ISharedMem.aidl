// ISharedMem.aidl
package com.ice.ashmem;

// Declare any non-default types here with import statements

import android.os.ParcelFileDescriptor;

interface ISharedMem {
    ParcelFileDescriptor getSharedFd();
}
