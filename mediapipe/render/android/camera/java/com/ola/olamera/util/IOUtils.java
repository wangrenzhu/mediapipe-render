package com.ola.olamera.util;

import androidx.annotation.RestrictTo;

import java.io.Closeable;

@RestrictTo(RestrictTo.Scope.LIBRARY_GROUP)
public class IOUtils {
    public static void safeClose(Closeable closeable) {
        if (closeable == null) {
            return;
        }
        try {
            closeable.close();
        } catch (Exception ignore) {
            CameraShould.fail();
        }
    }
}
