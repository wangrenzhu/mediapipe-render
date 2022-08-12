package com.ola.frameworks;

import android.content.Context;

import javax.microedition.khronos.egl.EGLContext;

public class OlaBeauty {

    static {
        System.loadLibrary("opipe_jni");
    }

    public native static long nativeInitAssertManager(Context context, String cacheDir);

    public native static long nativeCreate();

    public native static long nativeInitLut(long context, int width, int height, byte[] lutData);

    public native static void nativeRelease(long context);

    public native static void nativeInit(long context, byte[] data, long eglContext);

    public native static void nativeStartModule(long context);

    public native static void nativeStopModule(long context);

    public native static void nativeRenderTexture(long context, int width, int height, int textureId, long frameTime);

    public native static void nativeProcessVideoFrame(long context, int textureId, int width, int height, long frameTime);

    public native static void nativeProcessVideoFrameBytes(long context, byte[] data, int size, int width, int height, long frameTime);

}