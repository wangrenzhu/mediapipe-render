#include "SegmentationFilter.hpp"

namespace Opipe {
    const std::string kSegmentationFragmentShaderString = SHADER_STRING
    (
     varying highp vec2 vTexCoord;
     varying highp vec2 vTexCoord1;
     varying highp vec2 vTexCoord2;
     uniform sampler2D colorMap;
     uniform sampler2D colorMap1;
     uniform sampler2D colorMap2;
     
     void main()
     {
        lowp vec4 foreColor = texture2D(colorMap, vTexCoord);
        lowp vec4 maskColor = texture2D(colorMap1, vTexCoord1);
        lowp vec4 backgroundColor = texture2D(colorMap2, vTexCoord2);
//        lowp float factor = smoothstep(0.0, 1.0, maskColor.a);
//         lowp float factor = maskColor.a * maskColor.a;
        lowp float factor = maskColor.a;
        gl_FragColor = foreColor * factor + backgroundColor * (1.0 - factor);
     }
    );


    SegmentationFilter::SegmentationFilter(Context *context) : Filter(context) {
        _segmentationMask = nullptr;
        _backgroundImage = nullptr;
    }

    SegmentationFilter::~SegmentationFilter() {
        if (_segmentationMask) {
            _segmentationMask = nullptr;
        }
        if (_backgroundImage) {
            _backgroundImage->release();
            _backgroundImage = nullptr;
        }
    }

    bool SegmentationFilter::init(Context *context) {
        if (!Filter::initWithFragmentShaderString(context,
                                                  kSegmentationFragmentShaderString,
                                                  3)) {
            return false;
        }
        return true;
    }

    void SegmentationFilter::updateSegmentationMask(Framebuffer *maskbuffer) {
        setInputFramebuffer(maskbuffer, NoRotation, 1, true);
        if (_segmentationMask) {
            delete _segmentationMask;
            _segmentationMask = nullptr;
        }
        _segmentationMask = maskbuffer;
    }
    
    SegmentationFilter* SegmentationFilter::create(Context *context) {
        SegmentationFilter* ret = new (std::nothrow) SegmentationFilter(context);
        if (!ret || !ret->init(context)) {
            delete ret;
            ret = 0;
        }
        return ret;
    }

    void SegmentationFilter::setBackgroundImage(SourceImage *image) {
        if (_backgroundImage) {
            _backgroundImage->release();
            _backgroundImage = nullptr;
        }
        _backgroundImage = std::move(image);
        setInputFramebuffer(_backgroundImage->getFramebuffer(), NoRotation, 2, true);
    }

    bool SegmentationFilter::proceed(float frameTime, bool bUpdateTargets/* = true*/) {
        
        return Filter::proceed(frameTime, bUpdateTargets);
    }
}
