//
// Created by  jormin on 2021/4/30.
//

#ifndef OLA_EGLANDROID_H
#define OLA_EGLANDROID_H

#include <mutex>
#import <GLES3/gl3.h>
#include "android_hardware_buffer_compat.h"
#include "PlatformEGLAndroidCompat.h"

namespace Opipe {

    class EGLAndroid {
    public :
        static int getGLMajorVersion();

        static int getGLMinorVersion();

        static bool supportHardwareBuffer();

        static bool supportPBO();

    private :

        static void _initGLInfo();

        static std::mutex mMutex;

        static int mGLMajorVersion;
        static int mGLMinorVersion;


    }; 
}

#endif //OLA_EGLANDROID_H
