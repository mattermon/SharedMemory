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

#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#include "JniUtils.h"

#define SOCK_NAME "\0CTRL_SOCKET"

static SharedData *sData = nullptr;
static int sDataSize = sizeof(SharedData);
static int sFd = -1;

extern "C"
JNIEXPORT void JNICALL
Java_com_ice_ashmemreader_AshReaderHelper_init(JNIEnv *env, jclass clazz, jint jFd) {

    if (sFd > -1) {
        if (sData == nullptr) {
            LOGW("map zone size:%d", sDataSize);
            sData = (SharedData *) mmap(nullptr, sDataSize,
                                        PROT_READ, MAP_SHARED, sFd,
                                        0);
        }
    } else {
        LOGE("fd not initialized.");
    }

    LOGI("init end.");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ice_ashmemreader_AshReaderHelper_read(JNIEnv *env, jclass clazz) {
    if (sData == nullptr) {
        if (sFd < 0) {
            LOGE("error fd while read.");
            return;
        }
        LOGW("map zone size:%d", sDataSize);
        sData = (SharedData *) mmap(nullptr, sDataSize,
                                    PROT_READ, MAP_SHARED, sFd, 0);
    }
    LOGD("read num:%d str:%s", sData->num, sData->str);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ice_ashmemreader_AshReaderHelper_getFdBySocket(JNIEnv *env, jclass clazz) {
    LOGI("getFdBySocket");

    int data, sfd, fd;
    ssize_t nr;
    struct msghdr msgh;
    struct iovec iov;

    union {
        char buf[CMSG_SPACE(sizeof(int))];
        /* Space large enough to hold an 'int' */
        struct cmsghdr align;
    } controlMsg;
    struct cmsghdr *cmsgp;

//connect to peer socket
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    if (strlen(SOCK_NAME) < sizeof(addr.sun_path)) {
        memcpy(addr.sun_path, SOCK_NAME, sizeof(SOCK_NAME) - 1);
        LOGD("set sun_path:%s %s", addr.sun_path, &addr.sun_path[1]);
    } else {
        LOGE("sock path name too long");
        return;
    }

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1) {
        LOGE("open socket error");
        return;
    } else {
        LOGD("create socket");
    }

    if (connect(sfd, (struct sockaddr *) &addr, sizeof(addr.sun_family) + sizeof(SOCK_NAME) - 1) ==
        -1) {
        LOGE("connect error.");
        errno = -1;
        perror("connect:");
        close(sfd);
        return;
    }

    // receive msg
    msgh.msg_name = NULL;
    msgh.msg_namelen = 0;

    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    iov.iov_base = &data;
    iov.iov_len = sizeof(int);

    msgh.msg_control = controlMsg.buf;
    msgh.msg_controllen = sizeof(controlMsg.buf);

    LOGD("do recvmsg");
    nr = recvmsg(sfd, &msgh, 0);
    if (nr == -1) {
        LOGE("recvmsg error");
        return;
    }
    LOGI("recvmsg returned:%d", nr);

    cmsgp = CMSG_FIRSTHDR(&msgh);
    if (cmsgp == NULL || cmsgp->cmsg_len != CMSG_LEN(sizeof(int))) {
        LOGE("bad cmsg header / message length");
    }
    if (cmsgp->cmsg_level != SOL_SOCKET) {
        LOGE("cmsg_level != SOL_SOCKET");
    }
    if (cmsgp->cmsg_type != SCM_RIGHTS) {
        LOGE("cmsg_type != SCM_RIGHTS");
    }

    memcpy(&fd, CMSG_DATA(cmsgp), sizeof(int));
    LOGD("Received FD:%d", fd);
    sFd = fd;
}



