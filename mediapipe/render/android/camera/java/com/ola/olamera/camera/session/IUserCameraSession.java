package com.ola.olamera.camera.session;

public interface IUserCameraSession {

    enum State {
        ACTIVE,
        INACTIVE
    }


     boolean active();

     boolean inactive();

     boolean isActive();

}
