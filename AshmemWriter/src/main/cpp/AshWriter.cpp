#include <jni.h>
#include <android/log.h>

#include <string.h>
#include <stdio.h>
#include <cstdio>
#include <fcntl.h>
#include <linux/ashmem.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <cstddef>

/* Get common definition of System V style IPC.  */
#include <linux/ipc.h>

/* Get system dependent definition of `struct shmid_ds' and more.  */
#include <linux/shm.h>

#include "JniUtils.h"

#define ASH_DEV     "/dev/ashmem"
#define ASH_NAME_POSE   "testash"

static SharedData *sData;
static int sDataSize = sizeof(SharedData);

static int sFd = -1;

extern "C"
JNIEXPORT void JNICALL
Java_com_ice_sharedmemory_AshmemWriterHelper_initAshmem(JNIEnv *env, jclass clazz) {

    sFd = open(ASH_DEV, O_RDWR);
    if (sFd < 0) {
        LOGE("error open ashmem");
        return;
    }

    int ret = ioctl(sFd, ASHMEM_SET_NAME, ASH_NAME_POSE);
    if (ret < 0) {
        close(sFd);
        LOGE("error set name");
        return;
    }
    //set shared memory size
    ret = ioctl(sFd, ASHMEM_SET_SIZE, sDataSize);
    if (ret < 0) {
        close(sFd);
        LOGE("error set size");
        return;
    }
    LOGW("ash mem fd:%d", sFd);
    if (sData == nullptr) {
        LOGW("map data zone size:%d", sDataSize);
        sData = (SharedData *) mmap(nullptr, sDataSize,
                                    PROT_READ | PROT_WRITE, MAP_SHARED,
                                    sFd,
                                    0);
        memset(sData, 0, sDataSize);
    }

    LOGD("init end");
}


extern "C"
JNIEXPORT void JNICALL
Java_com_ice_sharedmemory_AshmemWriterHelper_write(JNIEnv *env, jclass clazz,
                                                   jint jNum, jstring jStr) {
    const auto *str = reinterpret_cast<const jchar *>(env->GetStringUTFChars(jStr, nullptr));

    sData->num = jNum;
    memcpy(sData->str, str, strlen(reinterpret_cast<const char *const>(str)));

    env->ReleaseStringChars(jStr, str);
}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_ice_sharedmemory_AshmemWriterHelper_getAshFd(JNIEnv *env, jclass clazz) {
    jintArray intArray = env->NewIntArray(3);
    jint *arr = env->GetIntArrayElements(intArray, NULL);
    arr[0] = sFd;
    env->ReleaseIntArrayElements(intArray, arr, 0);
    return intArray;
}
