package com.ola.olamerademo;
import com.ola.olamera.render.entry.RenderFlowData;
import com.ola.olamera.render.expansion.RenderExpansion;
import com.ola.frameworks.OlaBeauty;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executor;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


public class OlaBeautyWrapper extends RenderExpansion {
    private final OlaBeauty mBeauty;
    private Executor mGLExecutor;

    public void setGLExecutor(Executor GLExecutor) {
        mGLExecutor = GLExecutor;
        mBeauty.setExecutor(mGLExecutor);
    }

    public Executor getGLExecutor() {
        return mGLExecutor;
    }

    private final List<Runnable> mDoAfterSurfaceReady = new ArrayList<>();
    private boolean mIsSurfaceReady;


    public OlaBeautyWrapper() {
        mBeauty = new OlaBeauty();
        doAfterSurfaceReady(() -> mBeauty.init());

        //  mQStreamWrapper.doAfterSurfaceReady(()->{
        //     OlaBeauty.nativeInitAssertManager(this, getCacheDir().getAbsolutePath());
        //     byte[] data = getAssetBytes(getAssets(), "face_mesh_mobile_gpu.binarypb");
        //     OlaBeauty.nativeInit(graph, data, EGL14.eglGetCurrentContext());
        // });
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        mBeauty.onSurfaceCreated();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        mBeauty.onSurfaceChanged(width, height);
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
        TextureInfo outputTextureInfo = mBeauty.render(inputTextureInfo);
        RenderFlowData result = convert(outputTextureInfo);
        inputTextureInfo.recycle();
        outputTextureInfo.recycle();
        if (result == null) {
            return input;
        }
        return result;
    }

    private @NonNull
    TextureInfo convert(@NonNull RenderFlowData input, long timeStamp) {
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
        mBeauty.destroyInGLThread();
        mDoAfterSurfaceReady.clear();
    }


    public OlaBeauty unWrap() {
        return mBeauty;
    }
}
