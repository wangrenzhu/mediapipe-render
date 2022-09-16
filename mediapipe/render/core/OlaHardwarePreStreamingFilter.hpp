//
// Created by  jormin on 2021/5/5.
//
#ifndef OlaHardwarePreStreamingFilter_hpp
#define OlaHardwarePreStreamingFilter_hpp

#include "Filter.hpp"
#include "Context.hpp"
#include "OlaPreStreamingFilter.hpp"
#include "GLUtils.h"
#include "EGLAndroid.h"
#include "PlatformEGLAndroidCompat.h"
#include "android_hardware_buffer_compat.h"
#include "AndroidDirectAccessFrameBuffer.h"

namespace Opipe {

    class OlaHardwarePreStreamingFilter : public OlaPreStreamingFilter {

    public:
        static OlaHardwarePreStreamingFilter *create(Context *context);

        ~OlaHardwarePreStreamingFilter();

        bool init(Context *context) override;

        virtual Framebuffer *requestFrameBuffer(int width, int height) override;

        virtual void returnFrameBuffer() override;

    private:
        OlaHardwarePreStreamingFilter(Context *context);


    };

}


#endif
