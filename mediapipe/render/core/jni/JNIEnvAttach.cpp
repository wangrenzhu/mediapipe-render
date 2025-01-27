//
// Created by  jormin on 2021/7/1.
//

#include "JNIEnvAttach.h"
#include "mediapipe/framework/port/logging.h"

using namespace OpipeJNI;

//see https://stackoverflow.com/questions/27923917/cant-execute-javavm-detachcurrentthread-attempting-to-detach-while-still-r
JNIEnvAttach::JNIEnvAttach(JavaVM *javaVm) : mJavaVM(javaVm) {
    if (mJavaVM == nullptr) {
        return;
    }


    jint getEnvRes = mJavaVM->GetEnv((void **) &mJNIEnv, JNI_VERSION_1_6);

    if (JNI_EDETACHED == getEnvRes) {
        jint attachResult = mJavaVM->AttachCurrentThread(&mJNIEnv, nullptr);
        if (JNI_OK == attachResult) {
            mNewAttach = true;
            mHasAttach = true;
            LOG(INFO) << "###### JNIEnvAttach  attachOK :" << mHasAttach;
        } else {
            LOG(ERROR) << "###### JNIEnvAttach Failed to attach, cancel attachResult:" << attachResult;
            // Failed to attach, cancel
        }
    } else if (JNI_OK == getEnvRes) {
        // Current thread already attached, do not attach 'again' (just to save the attachedHere flag)
        // We make sure to keep mNewAttach Here = 0
        mHasAttach = true;
        LOG(INFO) << "###### JNIEnvAttach  attachOK :" << mHasAttach;
    } else {
        // JNI_EVERSION, specified version is not supported cancel this..
    }
    if (mJNIEnv == nullptr) {
        mHasAttach = false;
        mNewAttach = false;
    }

}


JNIEnvAttach::~JNIEnvAttach() {
    if (mHasAttach && mNewAttach) {
        mJavaVM->DetachCurrentThread(); // Done only when attachment was done here
    }
}