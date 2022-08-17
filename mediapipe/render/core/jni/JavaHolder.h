//
// Created by  jormin on 2021/7/1.
//

#ifndef Opipe_JAVAHOLDER_H
#define Opipe_JAVAHOLDER_H


#include "jni.h"
#include <mutex>


namespace OpipeJNI {

    /**
     * JavaObject的C++持有对象
     */
    class __attribute__((visibility("default"))) JavaHolder {
    public:

        explicit JavaHolder(JNIEnv *env, jobject object) {
            if (object) {
                javaObject = env->NewGlobalRef(object);
            }
        }

        /**
         * 获取java Object的时候，需要确保jobject没有给release掉
         * see release(JNIEnv *env)
         *
         * 可以通过releaseMutex来保证，也可以后续通过指针引用，保证可见性的情况下，通过jobject==nullptr判断
         */
        jobject getJObject() {
            return javaObject;
        }

        void release(JNIEnv *env);

        virtual ~JavaHolder();

        std::mutex releaseMutex;

        jobject javaObject = nullptr;

    };
}


#endif //Opipe_JAVAHOLDER_H
