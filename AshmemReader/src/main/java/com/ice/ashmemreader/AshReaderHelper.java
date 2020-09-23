package com.ice.ashmemreader;

/**
 * Created by zhangguopeng on 2020/9/9.
 */
public class AshReaderHelper {
    static {
        System.loadLibrary("ashreader");
    }

    public static native void init(int fds);
    public static native void read();
    public static native void getFdBySocket();
}
