package com.ola.frameworks;

import java.util.LinkedList;

public class TextureInfo {

    private TextureInfo() {
    }

    public int textureId = -1;
    public int textureWidth = 0;
    public int textureHeight = 0;
    public int deviceRotation = 0;
    public int textureRotation = 0;
    public long timestamp = 0l;
    public int afMode;
    public int afState;
    //...

    public Object extHolder;

    private final static int MAX_CACHE = 100;
    private final static LinkedList<TextureInfo> sCacheQueue = new LinkedList<>();

    public void recycle() {
        textureId = -1;
        textureWidth = 0;
        textureHeight = 0;
        deviceRotation = 0;
        textureRotation = 0;
        timestamp = 0;
        afMode = 0;
        afState = 0;
        extHolder = null;
        cache(this);
    }

    private static void cache(TextureInfo info) {
        synchronized (sCacheQueue) {
            if (sCacheQueue.size() < MAX_CACHE) {
                sCacheQueue.add(info);
            }
        }
    }

    public static TextureInfo obtain() {
        TextureInfo result;
        synchronized (sCacheQueue) {
            if (sCacheQueue.size() > 0) {
                result = sCacheQueue.remove();
            } else {
                result = new TextureInfo();
            }
        }
        return result;
    }


}
