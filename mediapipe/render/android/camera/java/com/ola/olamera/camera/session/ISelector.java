package com.ola.olamera.camera.session;

import java.util.List;

public interface ISelector {
    public List<String> filter(List<String> cameraIds);
}
