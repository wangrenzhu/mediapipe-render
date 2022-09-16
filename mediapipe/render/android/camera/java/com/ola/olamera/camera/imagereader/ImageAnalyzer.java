package com.ola.olamera.camera.imagereader;

import android.media.Image;

public interface ImageAnalyzer {
    public void analyze(Image image, int cameraSensorRotation, int imageRotation);
}
