package com.ola.beauty.demo;

import android.os.Handler;
import android.os.Looper;


import java.util.concurrent.Executor;
import java.util.concurrent.ScheduledExecutorService;

/**
 * Helper class for retrieving an {@link ScheduledExecutorService} which will post to the main
 * thread.
 *
 * <p>Since {@link ScheduledExecutorService} implements {@link Executor}, this can also be used
 * as a simple Executor.
 */
public final class MainThreadExecutor implements Executor {
    private static volatile MainThreadExecutor sInstance;

    private final Handler mMainHandler;

    private MainThreadExecutor() {
        mMainHandler = new Handler(Looper.getMainLooper());
    }

    public static MainThreadExecutor getInstance() {
        if (sInstance != null) {
            return sInstance;
        }
        synchronized (MainThreadExecutor.class) {
            sInstance = new MainThreadExecutor();
        }

        return sInstance;
    }

    @Override
    public void execute(Runnable command) {
        mMainHandler.post(command);
    }

}
