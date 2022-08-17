//
// Created by  jormin on 2021/7/1.
//

#include "JavaHolder.h"
#include <assert.h>

using namespace OpipeJNI;

void JavaHolder::release(JNIEnv *env) {
    std::unique_lock <std::mutex> lock(releaseMutex);
    if (javaObject != nullptr) {
        env->DeleteGlobalRef(javaObject);
        javaObject = nullptr;
    }
}

JavaHolder::~JavaHolder() {
    std::unique_lock <std::mutex> lock(releaseMutex);
    if (!javaObject) {
        assert("release java object global ref first");
    }
}