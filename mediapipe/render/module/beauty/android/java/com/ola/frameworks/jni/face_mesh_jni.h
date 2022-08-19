//
// Created by Felix Wang on 2022/8/8.
//

#ifndef OPIPE_FACE_MESH_JNI_H
#define OPIPE_FACE_MESH_JNI_H

#include <jni.h>
#include "mediapipe/render/core/jni/JavaObjectMap.hpp"
#include "mediapipe/render/module/beauty/face_mesh_module.h"

namespace OpipeJNI {
#ifdef __cplusplus
    extern "C" {
#endif  // __cplusplus

    #define OLA_METHOD(METHOD_NAME) \
      Java_com_ola_frameworks_OlaBeautyJNI_##METHOD_NAME
        
        struct DispatchTask {
                std::function<void(void)> func = nullptr;
            };

        typedef union {
            DispatchTask *p;
            jlong v = 0;
        } DispatchTaskPtr;

    // Creates a native opipe context.
        JNIEXPORT jlong JNICALL OLA_METHOD(nativeCreate) (JNIEnv *env, jobject thiz);

        JNIEXPORT jlong JNICALL OLA_METHOD(nativeInitAssertManager) (JNIEnv *env, jobject thiz, 
                                                                    jobject androidContext, jstring cacheDirPath);

        // Releases a native opipe context.
        JNIEXPORT void JNICALL OLA_METHOD(nativeRelease) (JNIEnv *env, jobject thiz, NativeId<Opipe::FaceMeshModule> instance);

        JNIEXPORT void JNICALL OLA_METHOD(nativeDoTask) (JNIEnv *env, jobject javaObject, 
                                                         NativeId<Opipe::FaceMeshModule> streamPtr, DispatchTaskPtr ptr);
        //init native opipe
        JNIEXPORT void JNICALL OLA_METHOD(nativeInit) (JNIEnv *env, jobject thiz, 
                                                       NativeId<Opipe::FaceMeshModule> instance,
                                                       jbyteArray data, jlong glContext);

        JNIEXPORT void JNICALL OLA_METHOD(nativeInitLut) (JNIEnv *env, jobject thiz, NativeId<Opipe::FaceMeshModule> instance, 
                                                         jobject whiten_bitmap);

        JNIEXPORT void JNICALL OLA_METHOD(nativeInitLut2) (JNIEnv *env, jobject thiz, NativeId<Opipe::FaceMeshModule> instance, 
                                                         jbyteArray data);


        JNIEXPORT void JNICALL OLA_METHOD(nativeStartModule) (JNIEnv* env, jobject thiz, NativeId<Opipe::FaceMeshModule> instance);

        JNIEXPORT void JNICALL OLA_METHOD(nativeStopModule) (JNIEnv* env, jobject thiz, NativeId<Opipe::FaceMeshModule> instance);

        JNIEXPORT jint JNICALL OLA_METHOD(nativeRenderTexture) (JNIEnv* env, jobject thiz, NativeId<Opipe::FaceMeshModule> instance, 
                                                                jint width, jint height, jint textureId, jlong frameTime);

        JNIEXPORT void JNICALL OLA_METHOD(nativeProcessVideoFrame) (JNIEnv* env, jobject thiz, NativeId<Opipe::FaceMeshModule> instance, 
                                                                   jint textureId, jint width, jint height,  jlong frameTime);

        JNIEXPORT void JNICALL OLA_METHOD(nativeProcessVideoFrameBytes) (JNIEnv* env, jobject thiz, NativeId<Opipe::FaceMeshModule> instance, 
                                                                        jbyteArray data, jint width, jint height, jlong frameTime);


        JNIEXPORT jint JNIOnLoad(JavaVM *vm, JNIEnv *env);



        /////////美颜参数相关
        /// @param smoothing 磨皮 0.0 - 1.0
        JNIEXPORT float JNICALL OLA_METHOD(nativeGetSmoothing)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance);
        /// 美白 0.0 - 1.0
        JNIEXPORT float JNICALL OLA_METHOD(nativeGetWhitening)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance);
        /// 瘦脸
        JNIEXPORT float JNICALL OLA_METHOD(nativeGetSlim)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance);
        ///大眼
        JNIEXPORT float JNICALL OLA_METHOD(nativeGetEye)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance);
        /// 瘦鼻
        JNIEXPORT float JNICALL OLA_METHOD(nativeGetNose)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance);
        ///分割
        JNIEXPORT bool JNICALL OLA_METHOD(nativeSegmentation)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance);
        
        JNIEXPORT void JNICALL OLA_METHOD(nativeSetSlim)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance,float slim);
        JNIEXPORT void JNICALL OLA_METHOD(nativeSetNose)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance,float nose);
        JNIEXPORT void JNICALL OLA_METHOD(nativeSetEye)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance,float eye);
        JNIEXPORT void JNICALL OLA_METHOD(nativeSetSmoothing)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance,float smoothing);
        JNIEXPORT void JNICALL OLA_METHOD(nativeSetWhitening)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance,float whitening);
        JNIEXPORT void JNICALL OLA_METHOD(nativeSegmentationEnable)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance,bool segEnable);
                
#ifdef __cplusplus
    }  // extern "C"
#endif  // __cplusplus
}

#endif //OPIPE_FACE_MESH_JNI_H
