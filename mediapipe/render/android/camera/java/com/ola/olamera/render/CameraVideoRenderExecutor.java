package com.ola.olamera.render;


import com.ola.olamera.render.view.AndroidGLSurfaceView;

import java.util.concurrent.Executor;

public class CameraVideoRenderExecutor implements Executor {

    private AndroidGLSurfaceView mGLSurfaceView;

    public CameraVideoRenderExecutor(AndroidGLSurfaceView GLSurfaceView) {
        mGLSurfaceView = GLSurfaceView;
    }


    @Override
    public void execute(Runnable command) {
        mGLSurfaceView.queueEvent(command);
    }
}
