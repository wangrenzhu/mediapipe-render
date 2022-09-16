package com.ola.olamera.camera.session;

import android.media.Image;

public interface InnerImageCaptureCallback {


    void onCaptureStart();

    void onCaptureSuccess(Image image);

    void onError(Exception e);

}
