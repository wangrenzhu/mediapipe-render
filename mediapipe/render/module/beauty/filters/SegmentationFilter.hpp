#ifndef SegmentationFilter_hpp
#define SegmentationFilter_hpp

#include "mediapipe/render/core/Framebuffer.hpp"
#include "mediapipe/render/core/Filter.hpp"
#include "mediapipe/render/core/SourceImage.hpp"
#include "mediapipe/render/core/Context.hpp"

namespace Opipe {
    class SegmentationFilter : public Opipe::Filter
    {
    public:
        SegmentationFilter(Context *context);
        virtual ~SegmentationFilter();

        static SegmentationFilter *create(Opipe::Context *context);
        bool init(Opipe::Context *context);
        
        virtual bool proceed(float frameTime = 0, bool bUpdateTargets = true) override;

        void updateSegmentationMask(Framebuffer *maskbuffer);

        void setBackgroundImage(SourceImage *image);

    private:
        Framebuffer *_segmentationMask = nullptr;
        SourceImage *_backgroundImage = nullptr;
    };
}

#endif
