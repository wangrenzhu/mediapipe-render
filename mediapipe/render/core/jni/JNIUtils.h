//
// Created by  jormin on 2021/6/2.
//

#ifndef Opipe_JNIUTILS_H
#define Opipe_JNIUTILS_H

#include <functional>
#include <thread>
#include <jni.h>
#include <android/bitmap.h>
#include <map>
#include <vector>

namespace OpipeJNI {

    class __attribute__((visibility("default"))) JNIUtils {
    public:

        static void putJavaHashMap(JNIEnv *env, jobject javaHashMap, const std::map<std::string, float> &map);

        static void putJavaHashMap(JNIEnv *env, jobject javaHashMap, const std::map<std::string, std::string> &map);


        static jobject createJavaHashMap(JNIEnv *env, int initCapactiy);

        static std::string JStrToStr(JNIEnv *env, jstring jstr);

        static std::string getClassName(JNIEnv *env, jclass cls);

        static jobject createBitmapJObject(JNIEnv *env, int width, int height);

        static int getIntObjectValue(JNIEnv *env, jobject object);

        static float getFloatObjectValue(JNIEnv *env, jobject object);

        static std::vector<float> getFloatArray(JNIEnv *env, jfloatArray jFloatArray);

        static jfloatArray  convert2JavaFloatArray(JNIEnv *env,const std::vector<float>& input);


        static std::string JByteArrayToString(JNIEnv *env, jbyteArray array) {
            std::string buf;
            if (array == nullptr) {
                return buf;
            }
            int len = env->GetArrayLength(array);
            if (len > 0) {
                buf.resize(len);
                env->GetByteArrayRegion(array, 0, len, (jbyte *) buf.data());
            }
            return buf;
        }


        static std::vector<char> JByteArrayToByteVector(JNIEnv *env, jbyteArray array) {
            auto buf = std::vector<char>();
            if (array == nullptr) {
                return buf;
            }
            int len = env->GetArrayLength(array);
            if (len > 0) {
                buf.resize(len);
                env->GetByteArrayRegion(array, 0, len, (jbyte *) buf.data());
            }
            return buf;
        }

        template<class T>
        static std::vector<T> JProtoVectorToNative(JNIEnv *env, jobjectArray object_array) {
            std::vector<T> result;
            size_t size = env->GetArrayLength(object_array);
            result.reserve(size);
            jmethodID mid_to_byte_array = 0;
            for (int i = 0; i < size; ++i) {
                jobject obj = env->GetObjectArrayElement(object_array, i);
                if (mid_to_byte_array == 0) {
                    jclass cls = env->GetObjectClass(obj);
                    mid_to_byte_array = env->GetMethodID(cls, "toByteArray", "()[B");
                    env->DeleteLocalRef(cls);
                }
                jbyteArray byte_array = (jbyteArray) env->CallObjectMethod(obj, mid_to_byte_array);
                T proto_object;
                proto_object.ParseFromString(JByteArrayToString(env, byte_array));
                env->DeleteLocalRef(byte_array);
                result.emplace_back(std::move(proto_object));
            }
            return result;
        }
    };


}

#endif //Opipe_JNIUTILS_H
