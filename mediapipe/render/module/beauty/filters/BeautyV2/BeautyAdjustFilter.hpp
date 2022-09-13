#ifndef BeautyAdjustFilter_hpp
#define BeautyAdjustFilter_hpp

#include "mediapipe/render/core/Filter.hpp"
#include "mediapipe/render/core/Context.hpp"

namespace Opipe
{
    class BeautyAdjustFilter : public Opipe::Filter
    {
    public:
        static BeautyAdjustFilter *create(Opipe::Context *context);
        bool init(Opipe::Context *context);
        void setIndensity(float indensity) { _indensity = indensity; };

        virtual bool proceed(float frameTime = 0, bool bUpdateTargets = true) override;

    public:
        BeautyAdjustFilter(Opipe::Context *context);
        ~BeautyAdjustFilter(){};
    private:
        float _indensity = 0.0;
    };
}

#endif