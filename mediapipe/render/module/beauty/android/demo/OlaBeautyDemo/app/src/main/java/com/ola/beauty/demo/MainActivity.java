package com.ola.beauty.demo;

import android.Manifest;
import android.content.pm.PackageManager;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Build;
import android.os.Bundle;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;


import com.ola.olamera.camera.concurrent.MainThreadExecutor;
import com.ola.olamera.render.view.CameraVideoView;

import java.io.IOException;
import java.io.InputStream;


public class MainActivity extends AppCompatActivity {

    private CameraVideoView mCameraVideoView;
    private ActivityCameraSession mActivityCameraSession;
    private OlaWrapper mOlaWrapper;

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

//        byte[] data = getAssetBytes(getAssets(), "face_mesh_mobile_gpu.binarypb");
//        mOlaWrapper = new OlaWrapper(getApplicationContext(), "face_mesh_mobile_gpu.binarypb",getImageFromAssetsFile("whiten.png"));
        mOlaWrapper = new OlaWrapper(getApplicationContext(), "face_mesh_mobile_gpu.binarypb","whiten.png");

        mCameraVideoView = new CameraVideoView(this, null);
        setContentView(mCameraVideoView);
        mActivityCameraSession = new ActivityCameraSession(this);
        mActivityCameraSession.setCameraPreview(mCameraVideoView);
        requestPermission(() -> mActivityCameraSession.onWindowCreate());

        mCameraVideoView.postDelayed(() -> {
            mCameraVideoView.getExpansionManager().addRenderExpansion(OlaWrapper.class, mOlaWrapper);
        }, 1000);


        mOlaWrapper.doAfterSurfaceReady(() -> mOlaWrapper.unWrap().doInit().addListener(() -> {
            mOlaWrapper.start(); //暂时自动开始
        }, MainThreadExecutor.getInstance()));
    }

    public Bitmap getImageFromAssetsFile(String fileName) {
        Bitmap bitmap = null;
        try {
            InputStream is = getAssets().open(fileName);
            BitmapFactory.Options options = new BitmapFactory.Options();
            bitmap = BitmapFactory.decodeStream(is, null, options);
            is.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return bitmap;
    }

    private Runnable mPermissionCacheRunnable;


    public static final int REQUEST_CODE = 1234;

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_CODE) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                if (mPermissionCacheRunnable != null) {
                    mPermissionCacheRunnable.run();
                    mPermissionCacheRunnable = null;
                }
            } else {
                finish();
            }
        }
    }


    @Override
    protected void onResume() {
        super.onResume();
        mActivityCameraSession.onWindowActive();
    }

    @Override
    protected void onPause() {
        super.onPause();
        mActivityCameraSession.onWindowInactive();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mActivityCameraSession.onWindowDestroy();
        mOlaWrapper.doAfterSurfaceReady(() -> {
            mOlaWrapper.stop();
            mOlaWrapper.unWrap().destroyInGLThread();
        });
    }

    /**
     * 请求授权
     */
    private void requestPermission(Runnable runnable) {

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) { //表示未授权时
            //进行授权
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA}, REQUEST_CODE);
            mPermissionCacheRunnable = runnable;
        } else {
            runnable.run();
        }
    }


}