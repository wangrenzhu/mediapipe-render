package com.ola.frameworks;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.annotation.WorkerThread;

import android.content.Context;

import javax.microedition.khronos.egl.EGLContext;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.Set;
import java.util.concurrent.Executor;

@Keep
public class OlaBeautyJNI {

    private Executor mExecutor;
    private long mNative;

    static {
        System.loadLibrary("opipe_jni");

    }

    public OlaBeautyJNI() {
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    public synchronized int init(@NonNull EGLContext context, Executor glExecutor) {
        if (context == null) {
            return 0;
        }
        try {
            loadLibrary();
        } catch (Exception e) {
            throw e;
        }
        long ptr = nativeInitContext(context.getNativeHandle());
        if (ptr == 0) {
            return 0;
        }
        mNative = ptr;
        mExecutor = glExecutor;
        return 1;
    }

    public boolean postNativeTask(long taskId) {
        if (mNative == 0) {
            return false;
        }
        mExecutor.execute(() -> {
            if (mNative == 0) {
                return;
            }
            nativeDoTask(mNative, taskId);
        });
        return true;
    }


    public synchronized boolean destroy() {
        if (mNative == 0) {
            return false;
        }

        nativeRelease(mNative);
        mNative = 0;
        mExecutor = null;
        return true;
    }

    public TextureInfo render(TextureInfo input) {
        if (input == null || input.textureId < 0) {
            return null;
        }
        if (mNative == 0) {
            return null;
        }
        return nativeRenderTexture(mNative, input.width, input.height, input.texture, input.frameTime);
    }

    public Executor getExecutor() {
        return mExecutor;
    }

    
    public native long nativeInitAssertManager(Context context, String cacheDir);

    public native long nativeCreate();

    public native long nativeInitLut(long context, int width, int height, byte[] lutData);

    @WorkerThread
    public native void nativeRelease(long context);

    @Keep
    private native void nativeDoTask(long nativeContext, long taskId);

    @WorkerThread
    public native void nativeInit(long context, byte[] data, long eglContext);

    public native void nativeStartModule(long context);

    public native void nativeStopModule(long context);

    @WorkerThread
    public native int nativeRenderTexture(long context, int width, int height, int textureId, long frameTime);

    @WorkerThread
    public native void nativeProcessVideoFrame(long context, int textureId, int width, int height, long frameTime);

    @WorkerThread
    public native void nativeProcessVideoFrameBytes(long context, byte[] data, int size, int width, int height, long frameTime);

}