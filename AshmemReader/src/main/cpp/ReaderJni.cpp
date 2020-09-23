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

    int data, lfd, sfd, fd, opt;
    ssize_t nr;
    bool useDatagramSocket;
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

//
//extern "C"
//JNIEXPORT void JNICALL
//Java_com_ice_ashmemreader_AshReaderHelper_receiveFd(JNIEnv *env, jclass clazz) {
//    //ref to: https://man7.org/tlpi/code/online/dist/sockets/scm_rights_recv.c.html
//    LOGI("begin receive fd");
//
//    int data, lfd, sfd, fd, opt;
//    ssize_t nr;
//    bool useDatagramSocket;
//    struct msghdr msgh;
//    struct iovec iov;
//
//    union {
//        char buf[CMSG_SPACE(sizeof(int))];
//        /* Space large enough to hold an 'int' */
//        struct cmsghdr align;
//    } controlMsg;
//    struct cmsghdr *cmsgp;
//
//    //unix bind
//    struct sockaddr_un addr;
//    memset(&addr, 0, sizeof(struct sockaddr_un));
//    addr.sun_family = AF_UNIX;
//    memcpy(addr.sun_path, SOCK_NAME, sizeof(SOCK_NAME) - 1);
//    lfd = socket(AF_UNIX, SOCK_STREAM, 0);
//    if (lfd == -1) {
//        LOGE("create socket error.");
//        return;
//    }
//    LOGD("do bind to file:%s %s", addr.sun_path, &addr.sun_path[1]);
////    if (bind(lfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
//    if (bind(lfd, (struct sockaddr *) &addr, sizeof(addr.sun_family) + sizeof(SOCK_NAME) - 1) ==
//        -1) {
//        close(lfd);
//        LOGE("bind error");
//        return;
//    }
//
//    //listen
//    LOGI("do listen");
//    if (listen(lfd, 5) == -1) {
//        LOGE("listen error.");
//        return;
//    }
//
//    unlink(SOCK_NAME);
//
//    LOGD("do accept");
//    sfd = accept(lfd, NULL, NULL);
//    if (sfd == -1) {
//        LOGE("accept error.");
//        return;
//    } else {
//        LOGD("accept %d", sfd);
//    }
//
//    // receive msg
//    msgh.msg_name = NULL;
//    msgh.msg_namelen = 0;
//
//    msgh.msg_iov = &iov;
//    msgh.msg_iovlen = 1;
//    iov.iov_base = &data;
//    iov.iov_len = sizeof(int);
//
//    msgh.msg_control = controlMsg.buf;
//    msgh.msg_controllen = sizeof(controlMsg.buf);
//
//    LOGD("do recvmsg");
//    nr = recvmsg(sfd, &msgh, 0);
//    if (nr == -1) {
//        LOGE("recvmsg error");
//        return;
//    }
//    LOGI("recvmsg returned:%d", nr);
//
//    cmsgp = CMSG_FIRSTHDR(&msgh);
//    if (cmsgp == NULL || cmsgp->cmsg_len != CMSG_LEN(sizeof(int))) {
//        LOGE("bad cmsg header / message length");
//    }
//    if (cmsgp->cmsg_level != SOL_SOCKET) {
//        LOGE("cmsg_level != SOL_SOCKET");
//    }
//    if (cmsgp->cmsg_type != SCM_RIGHTS) {
//        LOGE("cmsg_type != SCM_RIGHTS");
//    }
//
//    memcpy(&fd, CMSG_DATA(cmsgp), sizeof(int));
//    LOGD("Received FD:%d", fd);
//
//    sFd = fd;
//
//    Java_com_ice_ashmemreader_AshReaderHelper_init(env, clazz, fd);
//}
//
//int initSocket() {
//    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
//    if (sock == -1) {
//        LOGE("open socket error");
//        return -1;
//    }
//
//    struct sockaddr_un addr;
//    memset(&addr, 0, sizeof(addr));
//    addr.sun_family = AF_UNIX;
//    if (*SOCKET_PATH == '\0') {
//        LOGI("socket path is \\0");
//        *addr.sun_path = '\0';
//        strncpy(addr.sun_path + 1, SOCKET_PATH + 1, sizeof(addr.sun_path) - 2);
//    } else {
//        LOGI("socket path normal");
//        strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
//    }
//
//    LOGD("sock name:%s %s", addr.sun_path, &addr.sun_path[1]);
//
//    int ret = connect(sock, (struct sockaddr *) &addr, sizeof(addr));
//    if (ret == -1) {
//        LOGE("connect error.");
//        perror("connect error");
//        return 1;
//    }
//
//    char buf[40];
//    int *fds = nullptr;
//    read_fd(sock, buf, 40, fds);
//    LOGD("read fd:%d", fds[0]);
//}
//
//
//int read_fd(int sock, void *ptr, int nbytes, int *recvfd) {
//    LOGD("read_fd");
//    struct msghdr msg;
//    struct iovec iov[1];
//    int n;
//    int newfd;
////#ifdef HAVE_MSGHDR_MSG_CONTROL
//    union { // 对齐
//        struct cmsghdr cm;
//        char control[CMSG_SPACE(sizeof(int))];
//    } control_un;
//    struct cmsghdr *cmptr;
//    // 设置辅助数据缓冲区和长度
//    msg.msg_control = control_un.control;
//    msg.msg_controllen = sizeof(control_un.control);
////#else
////    msg.msg_accrights = (caddr_t) &newfd; // 这个简单
////    msg.msg_accrightslen = sizeof(int);
////#endif
//
//    // TCP无视
//    msg.msg_name = NULL;
//    msg.msg_namelen = 0;
//    // 设置数据缓冲区
//    iov[0].iov_base = ptr;
//    iov[0].iov_len = nbytes;
//    msg.msg_iov = iov;
//    msg.msg_iovlen = 1;
//    // 设置结束，准备接收
//    if ((n = recvmsg(sock, &msg, 0)) <= 0) {
//        return n;
//    }
////#ifdef HAVE_MSGHDR_MSG_CONTROL
//    // 检查是否收到了辅助数据，以及长度，回忆上一节的CMSG宏
//    cmptr = CMSG_FIRSTHDR(&msg);
//    if ((cmptr != NULL) && (cmptr->cmsg_len == CMSG_LEN(sizeof(int)))) {
//        // 还是必要的检查
//        if (cmptr->cmsg_level != SOL_SOCKET) {
//            printf("control level != SOL_SOCKET/n");
//            exit(-1);
//        }
//        if (cmptr->cmsg_type != SCM_RIGHTS) {
//            printf("control type != SCM_RIGHTS/n");
//            exit(-1);
//        }
//        // 好了，描述符在这
//        *recvfd = *((int *) CMSG_DATA(cmptr));
//    } else {
//        if (cmptr == NULL) printf("null cmptr, fd not passed./n");
//        else printf("message len[%d] if incorrect./n", cmptr->cmsg_len);
//        *recvfd = -1; // descriptor was not passed
//    }
////#else
////    if(msg.msg_accrightslen == sizeof(int)) *recvfd = newfd;
////    else *recvfd = -1;
////#endif
//    return n;
//}


