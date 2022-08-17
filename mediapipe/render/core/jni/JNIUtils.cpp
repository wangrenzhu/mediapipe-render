//
// Created by  jormin on 2021/6/2.
//
#include "JNIUtils.h"


namespace OpipeJNI {

    jobject JNIUtils::createJavaHashMap(JNIEnv *env, int initCapacty) {
        jclass mapClass = env->FindClass("java/util/HashMap");
        if(mapClass == nullptr){
            return nullptr;
        }
        jmethodID initMethod = env->GetMethodID(mapClass, "<init>", "(I)V");
        jobject hashMap = env->NewObject( mapClass, initMethod, initCapacty);
        env->DeleteLocalRef(mapClass);
        return hashMap;
    }

    void JNIUtils::putJavaHashMap(JNIEnv *env, jobject hashMap, const std::map<std::string, float> &map) {
        jclass mapClass = env->FindClass("java/util/HashMap");
        if (hashMap == nullptr || mapClass == nullptr) {
            return;
        }

        jmethodID put = env->GetMethodID(mapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
        jclass floatClass = env->FindClass("java/lang/Float");
        jmethodID floatValueOfMid = env->GetStaticMethodID(floatClass, "valueOf", "(F)Ljava/lang/Float;");

        auto citr = map.begin();
        for (; citr != map.end(); ++citr) {
            jstring keyJava = env->NewStringUTF(citr->first.c_str());
            jobject valueJava = env->CallStaticObjectMethod(floatClass, floatValueOfMid, citr->second);

            env->CallObjectMethod(hashMap, put, keyJava, valueJava);

            env->DeleteLocalRef(keyJava);
            env->DeleteLocalRef(valueJava);
        }

        env->DeleteLocalRef(mapClass);
        env->DeleteLocalRef(floatClass);
    }

    void JNIUtils::putJavaHashMap(JNIEnv *env, jobject hashMap, const std::map<std::string, std::string> &map) {
        jclass mapClass = env->FindClass("java/util/HashMap");
        if (hashMap == nullptr || mapClass == nullptr) {
            return;
        }
        jmethodID put = env->GetMethodID(mapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
        auto citr = map.begin();
        for (; citr != map.end(); ++citr) {
            jstring keyJava = env->NewStringUTF(citr->first.c_str());
            jstring valueJava = env->NewStringUTF(citr->second.c_str());

            env->CallObjectMethod(hashMap, put, keyJava, valueJava);

            env->DeleteLocalRef(keyJava);
            env->DeleteLocalRef(valueJava);
        }
        env->DeleteLocalRef(mapClass);
    }

    jfloatArray JNIUtils::convert2JavaFloatArray(JNIEnv *env,const std::vector<float> &input) {
        jfloatArray floatArrays = env->NewFloatArray(input.size());
        env->SetFloatArrayRegion(floatArrays, 0, input.size(), input.data());
        return floatArrays;
    }


    jobject JNIUtils::createBitmapJObject(JNIEnv *env, int width, int height) {
        //创建bitmap,耗时:1ms
        jclass bitmapConfig = env->FindClass("android/graphics/Bitmap$Config");
        jfieldID rgba8888FieldID = env->GetStaticFieldID(bitmapConfig, "ARGB_8888", "Landroid/graphics/Bitmap$Config;");
        jobject rgba8888Obj = env->GetStaticObjectField(bitmapConfig, rgba8888FieldID);

        jclass bitmapClass = env->FindClass("android/graphics/Bitmap");
        jmethodID createBitmapMethodID = env->GetStaticMethodID(bitmapClass, "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
        jobject newBitmap = env->CallStaticObjectMethod(bitmapClass, createBitmapMethodID, width, height, rgba8888Obj);
        return newBitmap;
    }

    std::string JNIUtils::JStrToStr(JNIEnv *env, jstring jstr) {
        std::string str;
        const char *s = env->GetStringUTFChars(jstr, JNI_FALSE);
        if (!s) {
            return str;
        }
        str = std::string(s);
        env->ReleaseStringUTFChars(jstr, s);
        return str;
    }

    std::string JNIUtils::getClassName(JNIEnv *env, jclass cls) {
        jclass clz = env->GetObjectClass(cls);
        jmethodID fid_name = env->GetMethodID(clz, "getName", "()Ljava/lang/String;");
        auto cls_name = reinterpret_cast<jstring>(env->CallObjectMethod(cls, fid_name));
        auto ret = JNIUtils::JStrToStr(env, cls_name);
        env->DeleteLocalRef(cls_name);
        env->DeleteLocalRef(clz);
        return ret;
    }

    int JNIUtils::getIntObjectValue(JNIEnv *env, jobject object) {
        assert(object != nullptr);
        jclass cls = env->FindClass("java/lang/Integer");
        jmethodID mid_get_value = env->GetMethodID(cls, "intValue", "()I");
        int i = env->CallIntMethod(object, mid_get_value);
        env->DeleteLocalRef(cls);
        return i;
    }

    float JNIUtils::getFloatObjectValue(JNIEnv *env, jobject object) {
        assert(object != nullptr);
        jclass cls = env->FindClass("java/lang/Float");
        jmethodID mid_get_value = env->GetMethodID(cls, "floatValue", "()F");
        float f = env->CallFloatMethod(object, mid_get_value);
        env->DeleteLocalRef(cls);
        return f;
    }

    std::vector<float> JNIUtils::getFloatArray(JNIEnv *env, jfloatArray jFloatArray) {
        std::vector<float> result;
        if (jFloatArray == nullptr) {
            return result;
        }
        jfloat *float_ptr = env->GetFloatArrayElements(jFloatArray, 0);
        const int length = env->GetArrayLength(jFloatArray);
        if (!float_ptr) {
            return result;
        }
        for (int i = 0; i < length; i++) {
            result.push_back(float_ptr[i]);
        }

        env->ReleaseFloatArrayElements(jFloatArray, float_ptr, 0);
        //值拷贝
        return result;

    }

}
