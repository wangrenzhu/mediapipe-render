package com.ola.beauty.demo;

import android.util.Log;

import androidx.annotation.NonNull;

import com.ola.olamera.render.entry.RenderFlowData;
import com.ola.olamera.render.expansion.RenderExpansion;
import com.ola.frameworks.OlaBeautyJNI;
import com.ola.frameworks.TextureInfo;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executor;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class OlaWrapper extends RenderExpansion {

    private Executor mGLExecutor;
    private long context;
    private OlaBeautyJNI beautyJNI;

    public OlaWrapper(long graph, OlaBeautyJNI beautyJNI) {
        this.context = graph;
        this.beautyJNI = beautyJNI;
    }

    public void setGLExecutor(Executor GLExecutor) {
        mGLExecutor = GLExecutor;
    }

    public Executor getGLExecutor() {
        return mGLExecutor;
    }

    private final List<Runnable> mDoAfterSurfaceReady = new ArrayList<>();
    private boolean mIsSurfaceReady;


    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {

    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        mIsSurfaceReady = true;
        for (Runnable runnable : mDoAfterSurfaceReady) {
            runnable.run();
        }
        mDoAfterSurfaceReady.clear();
    }

    public void doAfterSurfaceReady(Runnable runnable) {
        if (mIsSurfaceReady) {
            runnable.run();
        } else {
            mDoAfterSurfaceReady.add(runnable);
        }
    }


    @Override
    public @NonNull
    RenderFlowData render(@NonNull RenderFlowData input, long timestamp) {
        TextureInfo inputTextureInfo = convert(input, timestamp);
        Log.e("####", "###### inputTextureInfo = " + inputTextureInfo.textureId);
        beautyJNI.nativeProcessVideoFrame(context, inputTextureInfo.textureId, inputTextureInfo.textureWidth, inputTextureInfo.textureHeight, timestamp);
        int textureId = beautyJNI.nativeRenderTexture(context, input.textureWidth, input.textureHeight, inputTextureInfo.textureId, timestamp);
        input.texture = textureId;
        return input;
    }

    private @NonNull TextureInfo convert(@NonNull RenderFlowData input, long timeStamp) {
        TextureInfo texture = TextureInfo.obtain();
        texture.textureId = input.texture;
        texture.textureHeight = input.textureHeight;
        texture.textureWidth = input.textureWidth;
        texture.timestamp = timeStamp;
        texture.extHolder = input;
        return texture;
    }

    private RenderFlowData convert(@NonNull TextureInfo input) {
        if (input.extHolder instanceof RenderFlowData) {
            RenderFlowData inFlowData = (RenderFlowData) input.extHolder;
            RenderFlowData flowData = RenderFlowData.obtain(inFlowData.windowWidth, inFlowData.windowHeight, inFlowData.extraData);
            flowData.texture = input.textureId;
            flowData.textureHeight = input.textureHeight;
            flowData.textureWidth = input.textureWidth;
            return flowData;
        }
        return null;
    }


    @Override
    public void onSurfaceDestroy() {
        mDoAfterSurfaceReady.clear();
    }


//    public QStreaming unWrap() {
//        return mQStream;
//    }
}