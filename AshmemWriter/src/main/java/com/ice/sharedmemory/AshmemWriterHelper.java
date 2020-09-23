package com.ice.sharedmemory;

/**
 * Created by ice on 2020/9/9.
 */
public class AshmemWriterHelper {
    static {
        System.loadLibrary("ashwriter");
    }

    public static native void initAshmem();

    /**
     * @param num int型数据
     * @param str string数据
     */
    public static native void write(int num, String str);

    public static native void doWaitClient();

    public static native int[] getAshFd();
}
