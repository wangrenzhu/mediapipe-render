//
// Created by  jormin on 2021/5/5.
//

#include "OlaHardwarePreStreamingFilter.hpp"
#include "GPUImageUtil.h"

namespace Opipe {


OlaHardwarePreStreamingFilter::OlaHardwarePreStreamingFilter(Context *context)
        : Opipe::OlaPreStreamingFilter(context) {

}

OlaHardwarePreStreamingFilter::~OlaHardwarePreStreamingFilter() {
    if (_framebuffer) {
        delete _framebuffer;
        _framebuffer = nullptr;
    }
}


OlaHardwarePreStreamingFilter *OlaHardwarePreStreamingFilter::create(Context *context) {
    auto *ret = new(std::nothrow) OlaHardwarePreStreamingFilter(context);
    if (!ret || !ret->init(context)) {
        delete ret;
        ret = nullptr;
    }
    return ret;
}

bool OlaHardwarePreStreamingFilter::init(Context *context) {
    std::string input = std::string(kOlaPreStreamingFilterFragmentShaderString);
    std::string ess_ext_3_shader_string = GLUtils::ReplaceAllDistinct(input, "#version 300 es",
                                                                      "#version 300 es\n#extension GL_OES_EGL_image_external : require\n");
    if (!Opipe::Filter::initWithShaderString(context, std::string(kOlaPreStreamingFilterVertexShaderString),
                                                ess_ext_3_shader_string)) {
        Opipe::LogE("AHardwareBuffer", "init shader fail \n %s", ess_ext_3_shader_string.c_str());
        return false;
    } else {
        Opipe::Log("AHardwareBuffer", "init shader success (program:%d) %s", _filterProgram->getID(),ess_ext_3_shader_string.c_str());
    }
    return true;
}


Framebuffer *OlaHardwarePreStreamingFilter::requestFrameBuffer(int width, int height) {
    if (_framebuffer == nullptr) {
        _framebuffer = new AndroidDirectAccessFrameBuffer(_context, width, height,
                                                          Framebuffer::defaultTextureAttribures);
    } else {
        if(_framebuffer->getWidth() != width || _framebuffer->getHeight() != height) {
//            _framebuffer->release(false);
            delete _framebuffer;
            _framebuffer = 0;
            _framebuffer = new AndroidDirectAccessFrameBuffer(_context, width, height,
                                                              Framebuffer::defaultTextureAttribures);
        }
    }
    return _framebuffer;
}


void OlaHardwarePreStreamingFilter::returnFrameBuffer() {
    //not return framebuffer
}

}
