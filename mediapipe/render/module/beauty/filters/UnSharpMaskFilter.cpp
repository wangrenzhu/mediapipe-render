#include "UnSharpMaskFilter.hpp"

namespace Opipe {
    const std::string kUnsharpMaskFragmentShaderString = SHADER_STRING
    (
     varying highp vec2 vTexCoord;
     varying highp vec2 vTexCoord1;
     uniform sampler2D colorMap;
     uniform sampler2D colorMap1;
     uniform highp float intensity;
     void main()
     {
        lowp vec4 sharpImageColor = texture2D(colorMap, vTexCoord);
        lowp vec4 blurredImageColor = texture2D(colorMap1, vTexCoord1);
        gl_FragColor = vec4(sharpImageColor.rgb * intensity + blurredImageColor.rgb * (1.0 - intensity), blurredImageColor.a);
     }
   );
    
    class UnSharpFilter : public Filter {
    public:
        static UnSharpFilter* create(Context *context);
        bool init(Context *context);
        
        virtual bool proceed(float frameTime = 0.0, bool bUpdateTargets = true) override;
        void update(float frameTime = 0) override;
        void setIntensity(float intensity);
        
    protected:
        UnSharpFilter(Context *context);
        
        float _intensity;
    };
    
    UnSharpFilter::UnSharpFilter(Context *context) :
    Filter(context),
    _intensity(0.0) {
        
    }
    
    UnSharpFilter* UnSharpFilter::create(Context *context) {
        UnSharpFilter* ret = new (std::nothrow) UnSharpFilter(context);
        if (!ret || !ret->init(context)) {
            delete ret;
            ret = 0;
        }
        return ret;
    }
    
    bool UnSharpFilter::init(Context *context) {
        if (!Filter::initWithFragmentShaderString(context,
                                                  kUnsharpMaskFragmentShaderString,
                                                  2)) {
            return false;
        }
        return true;
    }

    void UnSharpFilter::update(float frameTime) {
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

        _framebuffer = firstInputFramebuffer;
        _framebuffer->lock();
        proceed(frameTime);
        _framebuffer->unlock();
        _framebuffer = 0;
    }
    
    void UnSharpFilter::setIntensity(float intensity) {
        _intensity = intensity;
    }
    
    bool UnSharpFilter::proceed(float frameTime, bool bUpdateTargets/* = true*/) {
        _filterProgram->setUniformValue("intensity", _intensity);
        return Filter::proceed(frameTime, bUpdateTargets);
    }
    
    UnSharpMaskFilter::UnSharpMaskFilter(Context *context)
    : FilterGroup(context) ,_blurFilter(0)
    ,_unsharpMaskFilter(0) {
        
    }

    UnSharpMaskFilter::~UnSharpMaskFilter() {
        if (_blurFilter) {
            _blurFilter->release();
            _blurFilter = 0;
        }
        
        if (_unsharpMaskFilter) {
            _unsharpMaskFilter->release();
            _unsharpMaskFilter = 0;
        }
    }

    UnSharpMaskFilter *UnSharpMaskFilter::create(Context *context) {
    UnSharpMaskFilter* ret = new (std::nothrow) UnSharpMaskFilter(context);
        if (!ret || !ret->init(context)) {
            delete ret;
            ret = 0;
        }
        return ret;
    }

    bool UnSharpMaskFilter::init(Context *context) {
        if (!FilterGroup::init(context)) {
            return false;
        }
        
        _blurFilter = GaussianBlurFilter::create(context);
        addFilter(_blurFilter);
        
        _unsharpMaskFilter = UnSharpFilter::create(context);
        addFilter(_unsharpMaskFilter);
        
        _blurFilter->addTarget(_unsharpMaskFilter,1);
        setTerminalFilter(_unsharpMaskFilter);
        
        return true;
    }

    void UnSharpMaskFilter::setIntensity(float intensity) {
        ((UnSharpFilter *)_unsharpMaskFilter)->setIntensity(intensity);
    }

    void UnSharpMaskFilter::setBlurRadiusInPixel(float blurRadius,
                                                              bool isVertical) {
        if (isVertical) {
            _blurFilter->setSigma_v(blurRadius);
        } else {
           _blurFilter->setSigma_h(blurRadius);
        }
    }
    
}
