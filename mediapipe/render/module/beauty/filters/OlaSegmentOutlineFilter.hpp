#ifndef OlaSegmentOutlineFilter_hpp
#define OlaSegmentOutlineFilter_hpp

#include "mediapipe/render/core/Filter.hpp"
#include "mediapipe/render/core/Context.hpp"

namespace Opipe
{
    class OlaSegmentOutlineFilter : public Opipe::Filter
    {
    public:
        static OlaSegmentOutlineFilter *create(Opipe::Context *context);
        bool init(Opipe::Context *context);

        virtual bool proceed(float frameTime = 0, bool bUpdateTargets = true) override;
        virtual void setInputFramebuffer(Framebuffer* framebuffer,
                                         RotationMode rotationMode = NoRotation,
                                         int texIdx = 0,
                                         bool ignoreForPrepared = false) override;

    public:
        OlaSegmentOutlineFilter(Opipe::Context *context);
        ~OlaSegmentOutlineFilter(){};

        float _opacityLimit;
    };
}

#endif
