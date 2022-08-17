//
// Created by  jormin on 2021/7/1.
//

#ifndef Opipe_JAVAOBJECTMAP_H
#define Opipe_JAVAOBJECTMAP_H

#include <jni.h>
#include "JavaHolder.h"
#include <vector>
#include <map>


namespace OpipeJNI {


    template<typename T>
    union __attribute__((visibility("default"))) NativeId {
        T *p;
        //必须设置默认值，32位机器指针长度不能填满long long类型
        jlong v = 0;
    };
}

namespace std {
    template<typename T>
    struct less<OpipeJNI::NativeId<T>> {
        bool operator()(const OpipeJNI::NativeId<T> &l, const OpipeJNI::NativeId<T> &r) const {
            return l.v > r.v;
        }
    };
}

namespace OpipeJNI {

    template<typename T>
    class __attribute__((visibility("default"))) JavaObjectMap {
    public:

        explicit JavaObjectMap() {

        }

        int addJavaObjectHolder(NativeId<T> id, std::unique_ptr<JavaHolder> holderPtr);


        JavaHolder *getJavaObjectHolder(NativeId<T> id);

        void clearJavaObjectHolder(JNIEnv *env, NativeId<T> id);


    private:
        std::map<NativeId<T>, std::unique_ptr<JavaHolder>> mHolderMap;
        std::mutex mMutex;
    };


    template<typename T>
    int JavaObjectMap<T>::addJavaObjectHolder(NativeId<T> id, std::unique_ptr<JavaHolder> holderPtr) {
        std::unique_lock<std::mutex> lk(mMutex);
        mHolderMap.emplace(id, std::move(holderPtr));
        return 1;
    }

    template<typename T>
    void JavaObjectMap<T>::clearJavaObjectHolder(JNIEnv *env, NativeId<T> id) {
        std::unique_lock<std::mutex> lk(mMutex);
        auto i = mHolderMap.find(id);
        if (i != mHolderMap.end()) {
            auto holder = i->second.get();
            holder->release(env);
            mHolderMap.erase(id);
        }
    }


    template<typename T>
    JavaHolder *JavaObjectMap<T>::getJavaObjectHolder(NativeId<T> id) {
        std::unique_lock<std::mutex> lk(mMutex);
        auto i = mHolderMap.find(id);
        if (i == mHolderMap.end()) {
            return nullptr;
        }
        return i->second.get();
    }
}


#endif //Opipe_JAVAOBJECTMAP_H
