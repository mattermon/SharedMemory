#include <jni.h>
#include <android/log.h>

#include <string.h>
#include <stdio.h>
#include <cstdio>
#include <fcntl.h>
#include <sys/socket.h>


#include <linux/ashmem.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <cstddef>

/* Get common definition of System V style IPC.  */
#include <linux/ipc.h>

/* Get system dependent definition of `struct shmid_ds' and more.  */
#include <linux/shm.h>

#include "JniUtils.h"

#include <sys/un.h>
#include <cstdlib>
#include <pthread.h>
#include <errno.h>

#define ASH_DEV     "/dev/ashmem"
#define ASH_NAME   "testash"

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

    int ret = ioctl(sFd, ASHMEM_SET_NAME, ASH_NAME);
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

#define SOCK_NAME "\0CTRL_SOCKET"

extern "C"
JNIEXPORT void JNICALL
Java_com_ice_sharedmemory_AshmemWriterHelper_doWaitClient(JNIEnv *env, jclass clazz) {
    LOGD("doWaitClient");
    int data, lfd, sfd;
    ssize_t ns;
    struct msghdr msgh;
    struct iovec iov;

    union {
        char buf[CMSG_SPACE(sizeof(int))];
        /* Space large enough to hold an 'int' */
        struct cmsghdr align;
    } controlMsg;
    struct cmsghdr *cmsgp;

    //build data
    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;

    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    iov.iov_base = &data;
    iov.iov_len = sizeof(int);
    data = 1234;
    LOGD("Sending data:%d", data);

    msgh.msg_control = controlMsg.buf;
    msgh.msg_controllen = sizeof(controlMsg.buf);
    memset(controlMsg.buf, 0, sizeof(controlMsg.buf));

    cmsgp = CMSG_FIRSTHDR(&msgh);
    cmsgp->cmsg_len = CMSG_LEN(sizeof(int));
    cmsgp->cmsg_level = SOL_SOCKET;
    cmsgp->cmsg_type = SCM_RIGHTS;
    memcpy(CMSG_DATA(cmsgp), &sFd, sizeof(int));

    //build data end

    //unix bind
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, SOCK_NAME, sizeof(SOCK_NAME) - 1);
    lfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (lfd == -1) {
        LOGE("create socket error.");
        return;
    }
    LOGD("do bind to file:%s %s", addr.sun_path, &addr.sun_path[1]);
    if (bind(lfd, (struct sockaddr *) &addr, sizeof(addr.sun_family) + sizeof(SOCK_NAME) - 1) ==
        -1) {
        close(lfd);
        LOGE("bind error");
        return;
    }

    //listen
    LOGI("do listen");
    if (listen(lfd, 5) == -1) {
        LOGE("listen error.");
        return;
    }

    unlink(SOCK_NAME);

    do {
        sfd = accept(lfd, NULL, NULL);
        if (sfd == -1) {
            LOGE("accept error.");
            return;
        } else {
            LOGD("accept %d", sfd);
        }

        ns = sendmsg(sfd, &msgh, 0);
        if (ns == -1) {
            LOGE("sendmsg error.");
            return;
        }

        LOGD("sendmsg returned:%d", ns);
        if (close(sfd) == -1) {
            LOGE("close fd error.");
            return;
        }
    } while (true);
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

