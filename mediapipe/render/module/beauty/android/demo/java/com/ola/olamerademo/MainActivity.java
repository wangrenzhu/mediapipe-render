package com.ola.olamerademo;

import android.Manifest;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.opengl.EGL14;
import android.os.Build;
import android.os.Bundle;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.google.common.io.ByteStreams;
import com.ola.olamera.render.view.CameraVideoView;
import com.ola.frameworks.OlaBeauty;

import java.io.IOException;
import java.io.InputStream;

@RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
public class MainActivity extends AppCompatActivity {

    private CameraVideoView mCameraVideoView;
    private ActivityCameraSession mActivityCameraSession;
    private QStreamWrapper mQStreamWrapper;
    private long graph;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        graph = OlaBeauty.nativeCreate();
        mQStreamWrapper = new QStreamWrapper(graph);
        mCameraVideoView = new CameraVideoView(this, null);
        setContentView(mCameraVideoView);
        mActivityCameraSession = new ActivityCameraSession(this);
        mActivityCameraSession.setCameraPreview(mCameraVideoView);
        requestPermission(() -> mActivityCameraSession.onWindowCreate());

        mCameraVideoView.postDelayed(() -> {
            mCameraVideoView.getExpansionManager().addRenderExpansion(QStreamWrapper.class, mQStreamWrapper);
        }, 1000);

        mQStreamWrapper.doAfterSurfaceReady(()->{
            OlaBeauty.nativeInitAssertManager(this, getCacheDir().getAbsolutePath());
            byte[] data = getAssetBytes(getAssets(), "face_mesh_mobile_gpu_prestream.binarypb");
            OlaBeauty.nativeInit(graph, data, EGL14.eglGetCurrentContext());
        });
    }

    public static byte[] getAssetBytes(AssetManager assets, String assetName) {
        byte[] assetData;
        try {
            InputStream stream = assets.open(assetName);
            assetData = ByteStreams.toByteArray(stream);
            stream.close();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        return assetData;
    }


    private Runnable mPermissionCacheRunnable;


    public static final int REQUEST_CODE = 1234;

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
        OlaBeauty.nativeStartModule(graph);
    }

    @Override
    protected void onPause() {
        super.onPause();
        mActivityCameraSession.onWindowInactive();
        OlaBeauty.nativeStopModule(graph);

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mActivityCameraSession.onWindowDestroy();
        OlaBeauty.nativeRelease(graph);
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