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
#include "face_mesh_jni.h"



using namespace OpipeJNI;

namespace OpipeJNI {
     struct DispatchTask {
        std::function<void(void)> func = nullptr;
    };

    typedef union {
        DispatchTask *p;
        jlong v = 0;
    } DispatchTaskPtr;


    static JavaObjectMap<Opipe::FaceMeshModule> gJavaObjectMap;


    static JavaVM *gVm;


    void dispatch_async_onjava(void *id, std::function<void(void)> func) {
        if (id == nullptr) {
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
                                                 jbyteArray data, jlong glContext)
    {
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
    }

    JNIEXPORT jlong JNICALL OLA_METHOD(nativeInitAssertManager) (JNIEnv *env, jobject thiz, jobject androidContext, jstring cacheDirPath)
    {
        mediapipe::AssetManager* asset_manager = Singleton<mediapipe::AssetManager>::get();
        
        const char* path_ref = env->GetStringUTFChars(cacheDirPath, nullptr);
        // Make a copy of the string and release the jni reference.
        std::string path_to(path_ref);
        env->ReleaseStringUTFChars(cacheDirPath, path_ref);
        return asset_manager->InitializeFromActivity(env, androidContext, path_to);
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
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        jbyte *data_ptr = env->GetByteArrayElements(lutData, nullptr);
        int size = env->GetArrayLength(lutData);
        faceModule->initLut(width, height, data_ptr, size);
        env->ReleaseByteArrayElements(lutData, data_ptr, JNI_ABORT);
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
        Opipe::FaceMeshModule *faceModule = (Opipe::FaceMeshModule *)instance.p;
        TextureInfo info;
        info.width = width;
        info.height = height;
        info.textureId = textureId;
        info.frameTime = frameTime;
        TextureInfo textureInfo = faceModule->renderTexture(std::move(info));
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

}