#include <jni.h>
#include <android/log.h>

#include <string>
#include <cstdio>
#include <fcntl.h>
#include <linux/ashmem.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <cstddef>
#include <unistd.h>
#include <sys/sem.h>

/* Get common definition of System V style IPC.  */
#include <linux/ipc.h>

#include "JniUtils.h"

static SharedData *sData = nullptr;
static int sDataSize = sizeof(SharedData);
static int sFd = -1;

extern "C"
JNIEXPORT void JNICALL
Java_com_ice_ashmemreader_AshReaderHelper_init(JNIEnv *env, jclass clazz, jint jFd) {


    sFd = jFd;

    if (sFd > -1) {
        if (sData == nullptr) {
            LOGW("map zone size:%d", sDataSize);
            sData = (SharedData *) mmap(nullptr, sDataSize,
                                        PROT_READ, MAP_SHARED, sFd,
                                        0);
        }
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ice_ashmemreader_AshReaderHelper_read(JNIEnv *env, jclass clazz) {

    LOGD("read num:%d str:%s", sData->num, sData->str);

}


