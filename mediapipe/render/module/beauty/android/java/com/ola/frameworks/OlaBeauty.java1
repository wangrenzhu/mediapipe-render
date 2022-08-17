package com.ola.frameworks;
import android.opengl.EGL14;
import android.opengl.EGLContext;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.Executor;
import java.util.concurrent.RejectedExecutionException;
import androidx.concurrent.futures.CallbackToFutureAdapter;

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

    public OlaBeauty() {

    }

    public void onSurfaceCreated() {

    }

    public void onSurfaceChanged(int width, int height) {

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
