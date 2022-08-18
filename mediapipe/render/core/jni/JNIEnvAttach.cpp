//
// Created by  jormin on 2021/7/1.
//

#include "JNIEnvAttach.h"
#include "mediapipe/framework/port/logging.h"

using namespace OpipeJNI;

//see https://stackoverflow.com/questions/27923917/cant-execute-javavm-detachcurrentthread-attempting-to-detach-while-still-r
JNIEnvAttach::JNIEnvAttach(JavaVM *javaVm) : mJavaVM(javaVm) {
    LOG(INFO) << "###### JNIEnvAttach mJavaVM:" << mJavaVM;
    if (mJavaVM == nullptr) {
        return;
    }


    jint getEnvRes = mJavaVM->GetEnv((void **) &mJNIEnv, JNI_VERSION_1_6);

    LOG(INFO) << "###### JNIEnvAttach getEnvRes:" << getEnvRes;
    if (JNI_EDETACHED == getEnvRes) {
        jint attachResult = mJavaVM->AttachCurrentThread(&mJNIEnv, nullptr);
        if (JNI_OK == attachResult) {
            mNewAttach = true;
            mHasAttach = true;
            LOG(INFO) << "###### JNIEnvAttach JNI_OK";
        } else {
            LOG(ERROR) << "###### JNIEnvAttach Failed to attach, cancel attachResult:" << attachResult;
            // Failed to attach, cancel
        }
    } else if (JNI_OK == getEnvRes) {
        // Current thread already attached, do not attach 'again' (just to save the attachedHere flag)
        // We make sure to keep mNewAttach Here = 0
        mHasAttach = true;
    } else {
        // JNI_EVERSION, specified version is not supported cancel this..
    }
    LOG(INFO) << "###### JNIEnvAttach mJNIEnv:" << mJNIEnv;
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