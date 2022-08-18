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



using namespace OpipeJNI;

namespace OpipeJNI {

    static JavaObjectMap<Opipe::FaceMeshModule> gJavaObjectMap;


    static JavaVM *gVm;


    void dispatch_async_onjava(void *id, std::function<void(void)> func) {
        if (id == nullptr) {
            LOG(INFO) << "###### dispatch_async_onjava id == nullptr";
            return;
        }
        LOG(INFO) << "###### dispatch_async_onjava before";
        NativeId<Opipe::FaceMeshModule> ptr;
        ptr.p = (Opipe::FaceMeshModule *) id;

        JavaHolder *holder = gJavaObjectMap.getJavaObjectHolder(ptr);
        LOG(INFO) << "###### dispatch_async_onjava holder" << holder;

        if (holder->getJObject() == nullptr) {
            LOG(INFO) << "###### dispatch_async_onjava holder->getJObject() == nullptr";
            return;
        }


        {
            JNIEnvAttach attach(gVm);

            if (!attach.isAttach()) {
                LOG(INFO) << "###### dispatch_async_onjava !attach.isAttach()";
                return;
            }
            LOG(INFO) << "###### dispatch_async_onjava new DispatchTask";
            auto *task = new DispatchTask();
            task->func = func;
            DispatchTaskPtr taskPtr;
            taskPtr.p = task;

            jclass java_render_clz = attach.getEnv()->GetObjectClass(holder->getJObject());
            jmethodID post_task_mid = attach.getEnv()->GetMethodID(java_render_clz, "postNativeTask", "(J)Z");


            bool result = attach.getEnv()->CallBooleanMethod(holder->getJObject(), post_task_mid, taskPtr.v);
            LOG(INFO) << "###### dispatch_async_onjava after result:" << result << " jclass" << java_render_clz;
            if (!result) {
                delete task;
            }
        }
    }

    JNIEXPORT jlong JNICALL OLA_METHOD(nativeCreate)(JNIEnv *env, jobject thiz)
    {
        LOG(INFO) << "###### nativeCreate before";
        Opipe::FaceMeshModule *faceModule = Opipe::FaceMeshModule::create();
        NativeId<Opipe::FaceMeshModule> ptr;
        ptr.p = faceModule;
        std::unique_ptr<JavaHolder> javaHolder = std::make_unique<JavaHolder>(env, thiz);
        gJavaObjectMap.addJavaObjectHolder(ptr, std::move(javaHolder));
        LOG(INFO) << "###### nativeCreate after: " << javaHolder.get() << "faceModule:" << faceModule;
        return reinterpret_cast<int64_t>(faceModule);
    }

    JNIEXPORT void JNICALL OLA_METHOD(nativeInit) (JNIEnv *env, jobject thiz, NativeId<Opipe::FaceMeshModule> instance, 
                                                 jbyteArray data, jlong glContext)
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
        faceModule->init(std::move(glThreadDispatch), glContext, data_ptr, size);
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
                                                     jint width, jint height, jbyteArray lutData) 
    {
        LOG(INFO) << "###### nativeInitLut before";
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        jbyte *data_ptr = env->GetByteArrayElements(lutData, nullptr);
        int size = env->GetArrayLength(lutData);
        faceModule->initLut(width, height, data_ptr, size);
        env->ReleaseByteArrayElements(lutData, data_ptr, JNI_ABORT);
        LOG(INFO) << "###### nativeInitLut after size:" << size;
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
        LOG(INFO) << "###### nativeRenderTexture texture:" << textureId;
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
                                                      DispatchTaskPtr ptr) {
        if (ptr.p) {
            if (ptr.p->func) {
                ptr.p->func();
            }
            delete ptr.p;
        }
    }
}

JNIEXPORT jint

JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    gVm = vm;
    JNIEnv *env = nullptr;
    LOG(INFO) << "###### JNI_OnLoad JNIEnv:" << env;
    if (vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {

        return JNI_VERSION_1_6;
    }

    return JNI_ERR;
}