package com.ola.frameworks;
import android.opengl.EGL14;
import android.opengl.EGLContext;
import java.util.ArrayList;
import java.util.HashMap;

import java.util.List;
import java.util.concurrent.Executor;
import java.util.concurrent.RejectedExecutionException;
import androidx.concurrent.futures.CallbackToFutureAdapter;

import android.graphics.Bitmap;

import com.google.common.io.ByteStreams;
import android.content.res.AssetManager;
import java.io.IOException;
import java.io.InputStream;

import com.google.common.util.concurrent.ListenableFuture;
import android.content.Context;

public class OlaBeauty {
    private Executor mExecutor;
    private ListenableFuture<Boolean> mDoInitFuture;
    private OlaBeautyJNI mNativeHandler;

        /**
     * 对mRenderTasks进行加载
     */
    private final Object mRenderTaskLock = new Object();

    private long mGLThreadId = -1;
    private Context mContext;

    private String mCacheDir;

    private String mGaphPath;
    private String mLutPath;

    private Bitmap mBitmap;

     /**
     * TODO
     * TODO 目前使用外部的GL mExecutor 是不可靠的，不可靠在于mExecutor不是一定会完成任务，有可能GL销毁了，会直接清空任务
     * TODO 所有需要后续加一个Wrapper 保证任务一定完成和在GL destroy的时候，回调所有pendding任务
     */
    public void setExecutor(Executor executor) {
        mExecutor = executor;
    }

    public void setContext(Context context) {
        mContext = context;
    }

    public void setCacheDir(String cacheDir) {
        mCacheDir = cacheDir;
    }

    public void setData(String path) {
        mGaphPath = path;
    }

    public ListenableFuture<Boolean> doInit() {
        if (mDoInitFuture != null) {
            return mDoInitFuture;
        }
        mDoInitFuture = CallbackToFutureAdapter.getFuture(completer -> {
            mExecutor.execute(() -> {
                boolean result = initInner();
                completer.set(result);
            });
            return "init Beauty";
        });
        return mDoInitFuture;
    }

    public OlaBeauty(Context context, String graphPath, Bitmap bitmap) {
        this.mContext = context;
        this.mCacheDir = context.getCacheDir().getAbsolutePath();
        this.mGaphPath = graphPath;
        this.mBitmap = bitmap;
    }

    public OlaBeauty(Context context, String graphPath, String lutPath) {
        this.mContext = context;
        this.mCacheDir = context.getCacheDir().getAbsolutePath();
        this.mGaphPath = graphPath;
        this.mLutPath = lutPath;
    }

    public void onSurfaceCreated() {

    }

    public void onSurfaceChanged(int width, int height) {

    }

    public void startModule() {
        if (mNativeHandler == null || mNativeHandler.getNative() == 0) {
            return;
        }

        mNativeHandler.nativeStartModule(mNativeHandler.getNative());
    }

    public void stopModule() {
        if (mNativeHandler == null || mNativeHandler.getNative() == 0) {
            return;
        }

        mNativeHandler.nativeStopModule(mNativeHandler.getNative());
    }

    public void processVideoFrame(TextureInfo input) {
        if (mNativeHandler == null || mNativeHandler.getNative() == 0) {
            return;
        }
        mNativeHandler.nativeProcessVideoFrame(mNativeHandler.getNative(), input.textureId, input.textureWidth, input.textureHeight, input.timestamp);
    }

    public byte[] getAssetBytes( String assetName) {
        byte[] assetData;
        try {
            InputStream stream = mContext.getAssets().open(assetName);
            assetData = ByteStreams.toByteArray(stream);
            stream.close();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        return assetData;
    }
    

    public TextureInfo render(TextureInfo input) {
        if (mNativeHandler == null || mNativeHandler.getNative() == 0) {
            return input;
        }
        TextureInfo result = mNativeHandler.render(input);
        if (result != null) {
            result.extHolder = input.extHolder;
            return result;
        } else {
            return input;
        }
    }

    private synchronized boolean initInner() {
        EGLContext eglContext = EGL14.eglGetCurrentContext();
        
        if (mNativeHandler != null) {
            return true;
        }
        OlaBeautyJNI beautyJNI = new OlaBeautyJNI();
        int result = beautyJNI.init(eglContext, mExecutor);
        if (result != 0) {
            mNativeHandler = beautyJNI;
            mNativeHandler.nativeInitAssertManager(mContext, mCacheDir);
            if(mBitmap !=null)
                mNativeHandler.nativeInitLut(mNativeHandler.getNative(), mBitmap);
            else if(mLutPath !=null)
                mNativeHandler.nativeInitLut2(mNativeHandler.getNative(), getAssetBytes(mLutPath));
            mNativeHandler.nativeInit(mNativeHandler.getNative(), getAssetBytes(mGaphPath), eglContext.getNativeHandle());
        }

        return false;

    }

    public boolean isGLThread() {
        return Thread.currentThread().getId() == mGLThreadId;
    }

    public void postGLThread(Runnable task) {
        synchronized (mRenderTaskLock) {
            mExecutor.execute(task);
        }
    }

    public void destroy() {
        mExecutor.execute(this::destroyInGLThread);
    }

    public synchronized void destroyInGLThread() {
        if (mNativeHandler != null) {
            mNativeHandler.destroy();
            mNativeHandler = null;
        }
    }
}
