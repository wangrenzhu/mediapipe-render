package com.ola.beauty.demo;

import android.Manifest;
import android.content.pm.PackageManager;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.SeekBar;
import android.widget.Switch;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;


import com.ola.olamera.camera.concurrent.MainThreadExecutor;
import com.ola.olamera.render.view.CameraVideoView;

import java.io.IOException;
import java.io.InputStream;


public class MainActivity extends AppCompatActivity implements SeekBar.OnSeekBarChangeListener {

    private CameraVideoView mCameraVideoView;
    private ActivityCameraSession mActivityCameraSession;
    private OlaWrapper mOlaWrapper;


    private SeekBar mSmoothSeekBar;
    private SeekBar mWhitenSeekBar;
    private SeekBar mSlimSeekBar;
    private SeekBar mEyeSeekBar;
    private SeekBar mNoseSeekBar;


    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

//        byte[] data = getAssetBytes(getAssets(), "face_mesh_mobile_gpu.binarypb");
//        mOlaWrapper = new OlaWrapper(getApplicationContext(), "face_mesh_mobile_gpu.binarypb",getImageFromAssetsFile("whiten.png"));
        mOlaWrapper = new OlaWrapper(getApplicationContext(), "face_mesh_mobile_vFlip.binarypb", "whiten.png");

        mCameraVideoView = new CameraVideoView(this, null);

        setContentView(R.layout.activity_main);
        ViewGroup view = findViewById(R.id.camera);
        view.addView(mCameraVideoView);
        initView();

//        setContentView(mCameraVideoView);
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

    private void initView() {
        ((SeekBar) findViewById(R.id.smoothing)).setOnSeekBarChangeListener(this);
        ((SeekBar) findViewById(R.id.whitening)).setOnSeekBarChangeListener(this);
        ((SeekBar) findViewById(R.id.slim)).setOnSeekBarChangeListener(this);
        ((SeekBar) findViewById(R.id.eye)).setOnSeekBarChangeListener(this);
        ((SeekBar) findViewById(R.id.nose)).setOnSeekBarChangeListener(this);

        mSmoothSeekBar = findViewById(R.id.smoothing);
        mWhitenSeekBar = findViewById(R.id.whitening);
        mSlimSeekBar = findViewById(R.id.slim);
        mEyeSeekBar = findViewById(R.id.eye);
        mNoseSeekBar = findViewById(R.id.nose);
//        ((Switch)findViewById(R.id.seg))
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

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        switch (seekBar.getId()) {
            case R.id.smoothing:
                float res = ((float) mSmoothSeekBar.getProgress()) / 100.0f;
                mOlaWrapper.unWrap().setSmoothing(res);
                break;
            case R.id.whitening:
                float res2 = ((float) mWhitenSeekBar.getProgress()) / 100.0f;
                mOlaWrapper.unWrap().setWhitening(res2);
                break;
            case R.id.slim:
                float res3 = ((float) mSlimSeekBar.getProgress()) / 100.0f;
                mOlaWrapper.unWrap().setSlim(res3);
                break;
            case R.id.eye:
                float res4 = ((float) mEyeSeekBar.getProgress()) / 100.0f;
                mOlaWrapper.unWrap().setEye(res4);
                break;
            case R.id.nose:
                float res5 = ((float) mNoseSeekBar.getProgress()) / 100.0f;
                mOlaWrapper.unWrap().setNose(res5);
                break;
            default:
                break;
        }
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {

    }
}