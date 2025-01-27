package com.ola.frameworks;

import android.content.Context;

import android.opengl.EGL14;
import android.opengl.EGLContext;
import android.graphics.Bitmap;

import com.ola.frameworks.TextureInfo;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.Set;
import java.util.concurrent.Executor;

public class OlaBeautyJNI {

    private Executor mExecutor;
    private long mNative;

    public OlaBeautyJNI() {
    }



    public synchronized int init(EGLContext context, Executor glExecutor) {
        if (context == null) {
            return 0;
        }
        try {
            System.loadLibrary("opipe_jni");
        } catch (Exception e) {
            throw e;
        }
        long ptr = nativeCreate();
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
        

        int textureId = nativeRenderTexture(mNative, input.textureWidth, input.textureHeight, 
                                            input.textureId, input.timestamp);

        TextureInfo output;
        output = input;
        output.textureId = textureId;
        return output;
    }

    public long getNative() {
        return mNative;
    }

    public Executor getExecutor() {
        return mExecutor;
    }

    public native long nativeInitAssertManager(Context context, String cacheDir);

    public native long nativeCreate();

    public native long nativeInitLut(long context, Bitmap bitmap, Bitmap greyBitmap);

    public native long nativeInitLutBytes(long context, byte[] data, byte[] greyData);

    public native void nativeRelease(long context);

    private native void nativeDoTask(long nativeContext, long taskId);

    public native void nativeInit(long context, byte[] data, long eglContext, boolean useBeautyV2);

    public native void nativeStartModule(long context);

    public native void nativeStopModule(long context);

    public native int nativeRenderTexture(long context, int width, int height, int textureId, long frameTime);

    public native void nativeProcessVideoFrame(long context, int textureId, int width, int height, long frameTime);

    public native void nativeProcessVideoFrameBytes(long context, byte[] data, int size, int width, int height,
            long frameTime);

    public native float nativeGetAvgRenderTime(long context);


    ///美颜参数
    public native float nativeGetSmoothing(long context);
    public native float nativeGetWhitening(long context);
    public native float nativeGetSlim(long context);
    public native float nativeGetEye(long context);
    public native float nativeGetNose(long context);
    public native boolean nativeSegmentation(long context);


    public native void nativeSetSlim(long context, float slim);
    public native void nativeSetNose(long context, float nose);
    public native void nativeSetEye(long context, float eye);
    public native void nativeSetSmoothing(long context, float smoothing);
    public native void nativeSetWhitening(long context, float whitening);
    public native void nativeUseSegmentation(long context, boolean segEnable);
    public native void nativeSetSegmentationBackgroud(long context, byte[] data);
    public native void nativeUseLandmarks(long context, boolean landmarksEnable);


}