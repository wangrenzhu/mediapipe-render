//
// Created by  jormin on 2021/4/30.
//

#ifndef OLA_ANDROIDDIRECTACCESSFRAMEBUFFER_H
#define OLA_ANDROIDDIRECTACCESSFRAMEBUFFER_H

#include "GPUImage-x.h"
#include "GLES/gl.h"
#include "GLES/glext.h"
#include "android/hardware_buffer.h"
#include "PlatformEGLAndroidCompat.h"
#include "android_hardware_buffer_compat.h"
#include "EGLAndroid.h"
#include "Framebuffer.hpp"


/**
 *  目前仅仅支持hardwarebuffer的高效读取，如果以后要支持upload的话，需要改造
 */

namespace Opipe {


    class AndroidDirectAccessFrameBuffer : public Framebuffer {
    public:


        AndroidDirectAccessFrameBuffer(Context *context, int width, int height,
                                       const TextureAttributes textureAttributes = defaultTextureAttribures);


        virtual ~AndroidDirectAccessFrameBuffer() override;

        void lockAddress() override;

        void unlockAddress() override;

        void *frameBufferGetBaseAddress() override;

        int getBytesPerRow() override;

        void _generateTexture() override;

        void _generateFramebuffer(bool needGenerateTexture = true) override;

        bool support() {
            return _support;
        }

    private :

        bool _generateHardwareBuffer();

        EGLImageKHR _imageEGL = EGL_NO_IMAGE_KHR;
        void *_hardwareBufferReadData = nullptr;

        AHardwareBuffer *_graphicBuf = nullptr;
        AHardwareBuffer_Desc *_graphicBufDes = nullptr;

        bool _support = false;

        bool _hasLock = false;


    };

}


#endif //OLA_ANDROIDDIRECTACCESSFRAMEBUFFER_H
