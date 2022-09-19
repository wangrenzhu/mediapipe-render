#include "AlphaBlendFilter.hpp"
#include "Context.hpp"

namespace Opipe {
    const std::string kAlphaBlendFragmentShaderString = SHADER_STRING
    (
         varying lowp vec2 vTexCoord;
         varying lowp vec2 vTexCoord1;
         uniform sampler2D colorMap;
         uniform sampler2D colorMap1;
         uniform lowp float mixturePercent;
         void main() {
             lowp vec4 textureColor = texture2D(colorMap, vTexCoord);
             lowp vec4 textureColor2 = texture2D(colorMap1, vTexCoord1);
             gl_FragColor = vec4(mix(textureColor.rgb, textureColor2.rgb, textureColor2.a * mixturePercent), textureColor.a);
         }
     );
    AlphaBlendFilter::AlphaBlendFilter(Context *context) : Filter(context), _mix(1.0) {
        
    }
    
    AlphaBlendFilter* AlphaBlendFilter::create(Context *context) {
        AlphaBlendFilter* ret = new (std::nothrow) AlphaBlendFilter(context);
        if (!ret || !ret->init(context)) {
            delete ret;
            ret = 0;
        }
        return ret;
    }
    
    bool AlphaBlendFilter::init(Context *context) {
        if (!Filter::initWithFragmentShaderString(context,
                                                  kAlphaBlendFragmentShaderString,
                                                  2)) {
            return false;
        }
        return true;
    }

    void AlphaBlendFilter::setInputFramebuffer(Framebuffer* framebuffer,
                                               RotationMode rotationMode,
                                               int texIdx, bool ignoreForPrepared) {
        Filter::setInputFramebuffer(framebuffer, rotationMode, texIdx, ignoreForPrepared);
    }

    bool AlphaBlendFilter::proceed(float frameTime,
                                          bool bUpdateTargets/* = true*/) {
        _filterProgram->setUniformValue("mixturePercent", _mix);
        return Filter::proceed(frameTime, bUpdateTargets);
    }

    void AlphaBlendFilter::update(float frameTime) {
        if (_inputFramebuffers.empty()) return;

        if (!_enable) {
            _framebuffer = _inputFramebuffers.begin()->second.frameBuffer;
            Source::updateTargets(frameTime);
            _framebuffer = 0;
            return;
        }

        Framebuffer* firstInputFramebuffer = _inputFramebuffers.begin()->second.frameBuffer;
        RotationMode firstInputRotation = _inputFramebuffers.begin()->second.rotationMode;
        if (!firstInputFramebuffer) return;

        int rotatedFramebufferWidth = firstInputFramebuffer->getWidth();
        int rotatedFramebufferHeight = firstInputFramebuffer->getHeight();
        if (rotationSwapsSize(firstInputRotation))
        {
            rotatedFramebufferWidth = firstInputFramebuffer->getHeight();
            rotatedFramebufferHeight = firstInputFramebuffer->getWidth();
        }

        if (_framebufferScale !=  1.0) {
            rotatedFramebufferWidth = int(rotatedFramebufferWidth * _framebufferScale);
            rotatedFramebufferHeight = int(rotatedFramebufferHeight * _framebufferScale);
        }
        _framebuffer = getContext()->getFramebufferCache()->fetchFramebuffer(_context, rotatedFramebufferWidth, rotatedFramebufferHeight);

        proceed(frameTime);
        _framebuffer = 0;
    }
//
//    void AlphaBlendFilter::update(float frameTime) {
//        if (_inputFramebuffers.empty()) return;
//
//        if (!_enable) {
//            _framebuffer = _inputFramebuffers.begin()->second.frameBuffer;
//            Source::updateTargets(frameTime);
//            _framebuffer = 0;
//            return;
//        }
//
//        Framebuffer* firstInputFramebuffer = _inputFramebuffers.begin()->second.frameBuffer;
//        RotationMode firstInputRotation = _inputFramebuffers.begin()->second.rotationMode;
//        if (!firstInputFramebuffer) return;
//
//        _framebuffer = firstInputFramebuffer;
//        _framebuffer->lock();
//        proceed(frameTime);
//        _framebuffer->unlock();
//        _framebuffer = 0;
//    }
}
