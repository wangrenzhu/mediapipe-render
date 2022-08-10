//
// Created by Felix Wang on 2022/8/8.
//

#include "face_mesh_jni.h"

JNIEXPORT jlong JNICALL OLA_METHOD(nativeCreate)(JNIEnv* env, jobject thiz) {
    return reinterpret_cast<int64_t>(Opipe::FaceMeshModule::create());
}

JNIEXPORT void JNICALL OLA_METHOD(nativeRelease)(JNIEnv* env, jobject thiz, jlong context){

}

JNIEXPORT void JNICALL OLA_METHOD(nativeInit)(JNIEnv* env,jobject thiz,jlong context, jbyteArray data, jlong glContext){
    
    Opipe::FaceMeshModule* faceModule =reinterpret_cast<Opipe::FaceMeshModule*>(context);
    jbyte* data_ptr = env->GetByteArrayElements(data, nullptr);
    int size = env->GetArrayLength(data);
    faceModule->init(reinterpret_cast<void*>(glContext), data_ptr, size);
    env->ReleaseByteArrayElements(data, data_ptr, JNI_ABORT);
}

JNIEXPORT void JNICALL OLA_METHOD(nativeStartModule)(JNIEnv* env, jobject thiz, jlong context){
    Opipe::FaceMeshModule* faceModule =reinterpret_cast<Opipe::FaceMeshModule*>(context);
    faceModule->startModule();
}

JNIEXPORT void JNICALL OLA_METHOD(nativeStopModule)(JNIEnv* env, jobject thiz, jlong context){
    Opipe::FaceMeshModule* faceModule =reinterpret_cast<Opipe::FaceMeshModule*>(context);
    faceModule->stopModule();
}


JNIEXPORT void JNICALL OLA_METHOD(nativeRenderTexture)(JNIEnv* env, jobject thiz, jlong context, jint width, jint height, jint textureId, jlong frameTime){
    Opipe::FaceMeshModule* faceModule =reinterpret_cast<Opipe::FaceMeshModule*>(context);
    TextureInfo info;
    info.width = width;
    info.height = height;
    info.textureId = textureId;
    info.frameTime = frameTime;
    faceModule->renderTexture(std::move(info));
}

JNIEXPORT void JNICALL OLA_METHOD(nativeProcessVideoFrame)(JNIEnv* env, jobject thiz, jlong context, jint textureId, jint width, jint height, jlong frameTime){
    Opipe::FaceMeshModule* faceModule =reinterpret_cast<Opipe::FaceMeshModule*>(context);
    faceModule->processVideoFrame(textureId, width, height, frameTime);
}

JNIEXPORT void JNICALL OLA_METHOD(nativeProcessVideoFrameBytes)(JNIEnv* env, jobject thiz, jlong context, jbyteArray data, jint width, jint height, jlong frameTime){
    Opipe::FaceMeshModule* faceModule =reinterpret_cast<Opipe::FaceMeshModule*>(context);
    jbyte* data_ptr = env->GetByteArrayElements(data, nullptr);
    int size = env->GetArrayLength(data);
    faceModule->processVideoFrame(reinterpret_cast<unsigned char*>(data_ptr), size, width, height, frameTime);
    env->ReleaseByteArrayElements(data, data_ptr, JNI_ABORT);
}



// static JNINativeMethod methods[] = {
//         {"create", "()J", reinterpret_cast<void*>(Java_com_ola_render_RenderJni_create)},
//         {"render", "(JIIIJZ)I", reinterpret_cast<void*>(Java_com_ola_render_RenderJni_render)},
//         {"release", "(J)V", reinterpret_cast<void*>(Java_com_ola_render_RenderJni_release)}
// };

// extern "C" 
// JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
//     JNIEnv* env;
//     if (JNI_OK != jvm->GetEnv(reinterpret_cast<void**> (&env),JNI_VERSION_1_6)) {
//         LOGE("JNI_OnLoad could not get JNI env");
//         return JNI_ERR;
//     }
//     jclass clazz = env->FindClass("com/ola/frameworks/OlaBeauty");  //获取Java NativeLib类
//     if (clazz == nullptr) {
//         LOGE ( "find class com.ola.frameworks.OlaBeauty failed\n");
//         return JNI_VERSION_1_6;
//     }
// 	//注册Native方法
//     if (env->RegisterNatives(clazz, methods, sizeof(methods)/sizeof((methods)[0])) < 0) {
//         LOGE("RegisterNatives error");
//         return JNI_ERR;
//     }
 
//     return JNI_VERSION_1_6;
// }

// extern "C" 
// JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
  
// }