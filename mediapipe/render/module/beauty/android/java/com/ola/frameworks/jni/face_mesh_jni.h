//
// Created by Felix Wang on 2022/8/8.
//

#ifndef OPIPE_FACE_MESH_JNI_H
#define OPIPE_FACE_MESH_JNI_H

#include <stdio.h>
#include <condition_variable>
#include "mediapipe/render/core/OlaContext.hpp"
#include "mediapipe/render/module/beauty/face_mesh_common.h"
#include "mediapipe/render/module/beauty/face_mesh_module.h"

#include <jni.h>

#ifdef __cplusplus
    extern "C" {
#endif  // __cplusplus

#define OLA_METHOD(METHOD_NAME) \
  Java_com_ola_frameworks_OlaBeauty_##METHOD_NAME

// Creates a native opipe context.
JNIEXPORT jlong JNICALL OLA_METHOD(nativeCreate)(JNIEnv* env, jobject thiz);

// Releases a native opipe context.
JNIEXPORT void JNICALL OLA_METHOD(nativeRelease)(JNIEnv* env, jobject thiz, jlong context);

//init native opipe
JNIEXPORT void JNICALL OLA_METHOD(nativeInit)(JNIEnv* env,jobject thiz,jlong context,jbyteArray data, jlong glContext);


JNIEXPORT void JNICALL OLA_METHOD(nativeStartModule)(JNIEnv* env, jobject thiz, jlong context);

JNIEXPORT void JNICALL OLA_METHOD(nativeStopModule)(JNIEnv* env, jobject thiz, jlong context);

JNIEXPORT void JNICALL OLA_METHOD(nativeRenderTexture)(JNIEnv* env, jobject thiz, jlong context, jint width, jint height, jint textureId, jlong frameTime);

JNIEXPORT void JNICALL OLA_METHOD(nativeProcessVideoFrame)(JNIEnv* env, jobject thiz, jlong context, jint textureId, jint width, jint height, jint step, jlong frameTime);

JNIEXPORT void JNICALL OLA_METHOD(nativeProcessVideoFrameBytes)(JNIEnv* env, jobject thiz, jlong context, jbyteArray data, jint width, jint height, jint step, jlong frameTime);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus


#endif //OPIPE_FACE_MESH_JNI_H
