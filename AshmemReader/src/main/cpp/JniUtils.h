//
// Created by ice on 2020/9/15.
//

#ifndef ANDROIDDEMOS_JNIUTILS_H
#define ANDROIDDEMOS_JNIUTILS_H

#define LOG_TAG "ash_reader"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

typedef struct innerData {
    int x;
    int y;
} innerData;

typedef struct _sharedData {
    innerData inner;
    char str[40];
} SharedData;

#endif //ANDROIDDEMOS_JNIUTILS_H
