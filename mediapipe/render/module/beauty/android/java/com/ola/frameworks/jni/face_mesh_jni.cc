//
// Created by Felix Wang on 2022/8/8.
//

#include <stdio.h>
#include <condition_variable>
#include <android/bitmap.h>
#include <android/log.h>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <deque>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "mediapipe/render/core/OlaContext.hpp"
#include "mediapipe/render/module/beauty/face_mesh_common.h"
#include "mediapipe/util/android/asset_manager_util.h"
#include "mediapipe/render/core/GLThreadDispatch.h"
#include "mediapipe/render/core/jni/JNIEnvAttach.h"
#include "mediapipe/framework/port/logging.h"
#include "face_mesh_jni.h"

#include <android/bitmap.h>

using namespace OpipeJNI;

namespace OpipeJNI {

    static JavaObjectMap<Opipe::FaceMeshModule> gJavaObjectMap;


    static JavaVM *gVm;

     void bitmap_to_mat(JNIEnv *env, jobject &srcBitmap, Opipe::OMat &srcMat) {
        void *srcPixels = nullptr;
        AndroidBitmapInfo srcBitmapInfo;
        AndroidBitmap_getInfo(env, srcBitmap, &srcBitmapInfo);
        AndroidBitmap_lockPixels(env, srcBitmap, &srcPixels);

        uint32_t srcHeight = srcBitmapInfo.height;
        uint32_t srcWidth = srcBitmapInfo.width;
        if (srcHeight == 0 || srcWidth == 0 || srcPixels == nullptr) {
            return;
        }
        srcMat = Opipe::OMat((int)srcWidth, (int)srcHeight, (int)srcBitmapInfo.stride);
        memcpy(srcMat.data, srcPixels, srcBitmapInfo.stride * srcBitmapInfo.height);
        AndroidBitmap_unlockPixels(env, srcBitmap);
        return;
    }


    void dispatch_async_onjava(void *id, std::function<void(void)> func) {
        if (id == nullptr) {
            LOG(ERROR) << "###### dispatch_async_onjava id == nullptr";
            return;
        }
        NativeId<Opipe::FaceMeshModule> ptr;
        ptr.p = (Opipe::FaceMeshModule *) id;

        JavaHolder *holder = gJavaObjectMap.getJavaObjectHolder(ptr);
        if (holder->getJObject() == nullptr) {
            return;
        }


        {
            JNIEnvAttach attach(gVm);

            if (!attach.isAttach()) {
                LOG(ERROR) << "###### dispatch_async_onjava  ERROR !attach.isAttach() gVM:" << gVm;
                return;
            }
            auto *task = new DispatchTask();
            task->func = func;
            DispatchTaskPtr taskPtr;
            taskPtr.p = task;

            jclass java_render_clz = attach.getEnv()->GetObjectClass(holder->getJObject());
            jmethodID post_task_mid = attach.getEnv()->GetMethodID(java_render_clz, "postNativeTask", "(J)Z");


            bool result = attach.getEnv()->CallBooleanMethod(holder->getJObject(), post_task_mid, taskPtr.v);
            
            if (!result) {
                LOG(ERROR) << "###### dispatch_async_onjava after result:" << result << " jclass" << java_render_clz;
                delete task;
            }
        }
    }

    JNIEXPORT jlong JNICALL OLA_METHOD(nativeCreate)(JNIEnv *env, jobject thiz)
    {
        Opipe::FaceMeshModule *faceModule = Opipe::FaceMeshModule::create();
        NativeId<Opipe::FaceMeshModule> ptr;
        ptr.p = faceModule;
        std::unique_ptr<JavaHolder> javaHolder = std::make_unique<JavaHolder>(env, thiz);
        gJavaObjectMap.addJavaObjectHolder(ptr, std::move(javaHolder));
        return reinterpret_cast<int64_t>(faceModule);
    }

    JNIEXPORT void JNICALL OLA_METHOD(nativeInit) (JNIEnv *env, jobject thiz, NativeId<Opipe::FaceMeshModule> instance, 
                                                 jbyteArray data, jlong glContext, jboolean useBeautyV2)
    {
        LOG(INFO) << "###### nativeInit before";
        //获取GL版本信息
        const GLubyte *byteGlVersion = glGetString(GL_VERSION);

        GLint majVers, minVers;
        glGetIntegerv(GL_MAJOR_VERSION, &majVers);
        glGetIntegerv(GL_MINOR_VERSION, &minVers);
        std::thread::id glThreadId = std::this_thread::get_id();
        auto *glThreadDispatch = new Opipe::GLThreadDispatch(glThreadId, dispatch_async_onjava);

        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        jbyte *data_ptr = env->GetByteArrayElements(data, nullptr);
        int size = env->GetArrayLength(data);
        faceModule->init(std::move(glThreadDispatch), glContext, data_ptr, size, useBeautyV2);
        env->ReleaseByteArrayElements(data, data_ptr, JNI_ABORT);
        LOG(INFO) << "###### nativeInit after glThreadId:" << glThreadId;
    }

    JNIEXPORT jlong JNICALL OLA_METHOD(nativeInitAssertManager) (JNIEnv *env, jobject thiz, jobject androidContext, jstring cacheDirPath)
    {
        LOG(INFO) << "###### nativeInitAssertManager before";
        mediapipe::AssetManager* asset_manager = Singleton<mediapipe::AssetManager>::get();
        
        const char* path_ref = env->GetStringUTFChars(cacheDirPath, nullptr);
        // Make a copy of the string and release the jni reference.
        std::string path_to(path_ref);
        env->ReleaseStringUTFChars(cacheDirPath, path_ref);
        jlong rs =  asset_manager->InitializeFromActivity(env, androidContext, path_to);
        LOG(INFO) << "###### nativeInitAssertManager after" << rs;
        return rs;
    }

    JNIEXPORT void JNICALL OLA_METHOD(nativeRelease)(JNIEnv *env, jobject thiz, NativeId<Opipe::FaceMeshModule> instance)
    {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        gJavaObjectMap.clearJavaObjectHolder(env, instance);
        delete instance.p;
    }

    JNIEXPORT void JNICALL OLA_METHOD(nativeInitLut)(JNIEnv *env, jobject thiz, NativeId<Opipe::FaceMeshModule> instance, 
                                                     jobject whiten_bitmap, jobject grey_bitmap) 
    {
        LOG(INFO) << "###### nativeInitLut before";
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        Opipe::OMat lutMat;
        Opipe::OMat greyMat;
        bitmap_to_mat(env, whiten_bitmap, lutMat);
        if(grey_bitmap!= nullptr){
            bitmap_to_mat(env, grey_bitmap, greyMat);
        }
        faceModule->initLut(lutMat, greyMat);
        env->DeleteLocalRef(whiten_bitmap);
        if(grey_bitmap!= nullptr){
            env->DeleteLocalRef(grey_bitmap);
        }
        LOG(INFO) << "###### nativeInitLut after";
    }

    JNIEXPORT void JNICALL OLA_METHOD(nativeInitLutBytes) (JNIEnv *env, jobject thiz, NativeId<Opipe::FaceMeshModule> instance, 
                                                         jbyteArray data, jbyteArray greyData)
    {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        jbyte *data_ptr = env->GetByteArrayElements(data, nullptr);
        int size = env->GetArrayLength(data);
        int width, height, channels_in_file;
        auto pixdata = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(data_ptr),
                                    size, &width, &height,
                                    &channels_in_file, 4);

        if (!pixdata) {
            LOG(ERROR) << "stbi_load_from_memory failed";
            return;
        }
        Opipe::OMat lutMat = Opipe::OMat(width, height,std::move(reinterpret_cast<char*>(pixdata)));


        int grey_width, grey_height, grey_channels_in_file;
        Opipe::OMat greyMat;
        jbyte *grey_data_ptr;
        if(greyData != nullptr){
            grey_data_ptr = env->GetByteArrayElements(greyData, nullptr);
            int grey_size = env->GetArrayLength(greyData);
            
            auto grey_pixdata = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(grey_data_ptr),
                                    grey_size, &grey_width, &grey_height,
                                    &grey_channels_in_file, 4);
            greyMat = Opipe::OMat(grey_width, grey_height,std::move(reinterpret_cast<char*>(grey_pixdata)));
        }
       
        faceModule->initLut(lutMat, greyMat);
        env->ReleaseByteArrayElements(data, data_ptr, JNI_ABORT);
        
        if(greyData != nullptr){
            env->ReleaseByteArrayElements(greyData, grey_data_ptr, JNI_ABORT);
        }
    }
                                                

    JNIEXPORT void JNICALL OLA_METHOD(nativeStartModule) (JNIEnv *env, jobject thiz, NativeId<Opipe::FaceMeshModule> instance)
    {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        faceModule->startModule();
    }

    JNIEXPORT void JNICALL OLA_METHOD(nativeStopModule) (JNIEnv *env, jobject thiz, NativeId<Opipe::FaceMeshModule> instance)
    {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        faceModule->stopModule();
    }

    JNIEXPORT jint JNICALL OLA_METHOD(nativeRenderTexture) (JNIEnv *env, jobject thiz, NativeId<Opipe::FaceMeshModule> instance, 
    jint width, jint height, jint textureId, jlong frameTime) 
    {
        LOG(INFO) << "###### before nativeRenderTexture texture:" << textureId;
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        TextureInfo info;
        info.width = width;
        info.height = height;
        info.textureId = textureId;
        info.frameTime = frameTime;
        TextureInfo textureInfo = faceModule->renderTexture(info);
        LOG(INFO) << "###### after nativeRenderTexture texture:" << textureInfo.textureId;
        return textureInfo.textureId;
    }

    JNIEXPORT void JNICALL OLA_METHOD(nativeProcessVideoFrame) (JNIEnv *env, jobject thiz, 
            NativeId<Opipe::FaceMeshModule> instance, jint textureId, jint width, jint height, jlong frameTime) 
    {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        faceModule->processVideoFrame(textureId, width, height, frameTime);
    }

    JNIEXPORT void JNICALL OLA_METHOD(nativeProcessVideoFrameBytes) (JNIEnv *env, jobject thiz, 
            NativeId<Opipe::FaceMeshModule> instance, jbyteArray data, jint width, jint height, jlong frameTime) {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        jbyte *data_ptr = env->GetByteArrayElements(data, nullptr);
        int size = env->GetArrayLength(data);
        faceModule->processVideoFrame(reinterpret_cast<unsigned char*>(data_ptr), size, width, height, frameTime);
        env->ReleaseByteArrayElements(data, data_ptr, JNI_ABORT);
    }    


     JNIEXPORT void JNICALL OLA_METHOD(nativeDoTask) (JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> streamPtr, 
                                                      DispatchTaskPtr ptr)
    {
        if (ptr.p) {
            if (ptr.p->func) {
                ptr.p->func();
            }
            delete ptr.p;
        }
    }

    //////////////////////////////////////
    ///////美颜参数相关
    JNIEXPORT jfloat JNICALL OLA_METHOD(nativeGetSmoothing)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance) {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        return faceModule->getSmoothing();
    }
    /// 美白 0.0 - 1.0
    JNIEXPORT jfloat JNICALL OLA_METHOD(nativeGetWhitening)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance) {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        return faceModule->getWhitening();
    }
    /// 瘦脸
    JNIEXPORT jfloat JNICALL OLA_METHOD(nativeGetSlim)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance) {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        return faceModule->getSlim();
    }
    ///大眼
    JNIEXPORT jfloat JNICALL OLA_METHOD(nativeGetEye)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance) {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        return faceModule->getEye();
    }
    /// 瘦鼻
    JNIEXPORT jfloat JNICALL OLA_METHOD(nativeGetNose)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance) {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        return faceModule->getNose();
    }
    ///分割
    JNIEXPORT jboolean JNICALL OLA_METHOD(nativeGetSegmentation)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance) {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        return faceModule->getSegmentation();
    }
    
    JNIEXPORT void JNICALL OLA_METHOD(nativeSetSlim)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance, jfloat slim) {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        faceModule->setSlim(slim);
    }
    JNIEXPORT void JNICALL OLA_METHOD(nativeSetNose)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance, jfloat nose) {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        faceModule->setNose(nose);
    }
    JNIEXPORT void JNICALL OLA_METHOD(nativeSetEye)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance, jfloat eye) {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        faceModule->setEye(eye);
    }
    JNIEXPORT void JNICALL OLA_METHOD(nativeSetSmoothing)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance, jfloat smoothing) {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        faceModule->setSmoothing(smoothing);
    }
    JNIEXPORT void JNICALL OLA_METHOD(nativeSetWhitening)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance, jfloat whitening) {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        faceModule->setWhitening(whitening);
    }
    JNIEXPORT void JNICALL OLA_METHOD(nativeUseSegmentation)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance, jboolean segEnable) {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        faceModule->setSegmentationEnable(segEnable);
    }
    JNIEXPORT void JNICALL OLA_METHOD(nativeUseLandmarks)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance, jboolean landmarkEnable) {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        faceModule->setLandmarksEnable(landmarkEnable);
    }
                
    JNIEXPORT void JNICALL OLA_METHOD(nativeSetSegmentationBackgroud)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance, jbyteArray data) {
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        jbyte *data_ptr = env->GetByteArrayElements(data, nullptr);
        int size = env->GetArrayLength(data);
        
        int width, height, channels_in_file;
        auto pixdata = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(data_ptr),
                                    size, &width, &height,
                                    &channels_in_file, 4);
        if (!pixdata) {
            LOG(ERROR) << "nativeSetSegmentationBackgroud stbi_load_from_memory failed";
            return;
        }

        LOG(INFO) << "nativeSetSegmentationBackgroud width:" << width << " height:" << height;

        Opipe::OMat bgMat = Opipe::OMat(width, height, std::move(reinterpret_cast<char*>(pixdata)));

        faceModule->setSegmentationBackground(bgMat);
        env->ReleaseByteArrayElements(data, data_ptr, JNI_ABORT);
    }

JNIEXPORT jfloat JNICALL OLA_METHOD(nativeGetAvgRenderTime)(JNIEnv *env, jobject javaObject, NativeId<Opipe::FaceMeshModule> instance){
    Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
    return faceModule->currentProfile().avgRenderTime;
}


}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    gVm = vm;
    JNIEnv *env = nullptr;
    LOG(INFO) << "###### JNI_OnLoad" << vm;
    if (vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
        LOG(INFO) << "###### JNI_OnLoad SUCCESS gVM:" << gVm;
        return JNI_VERSION_1_6;
    }
    LOG(ERROR) << "###### JNI_OnLoad JNI_ERR:" << vm;

    return JNI_ERR;
}
