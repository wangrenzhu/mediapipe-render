//
// Created by  jormin on 2021/7/1.
//

#ifndef Opipe_JNIENVATTACH_H
#define Opipe_JNIENVATTACH_H

#include <jni.h>

namespace OpipeJNI {

    /**
     * 对象在构造时候尝试attach JNIEnv到当前线程，当对象释放的时候，如果之前线程本身没有JNIEnv环境，那么会Detach掉JNIEnv
     */
    class __attribute__((visibility("default"))) JNIEnvAttach {
    public :
        JNIEnvAttach(JavaVM *javaVm);

        bool isAttach() {
            return mHasAttach;
        }

        JNIEnv *getEnv() {
            return mJNIEnv;
        }

        ~JNIEnvAttach();

    private :
        bool mNewAttach = false; // know if detaching at the end is necessary
        bool mHasAttach = false;
        JNIEnv *mJNIEnv = nullptr;
        JavaVM *mJavaVM = nullptr;
    };
}


#endif //Opipe_JNIENVATTACH_H
