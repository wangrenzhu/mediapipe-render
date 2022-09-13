#ifndef BeautyHighPassFilter_hpp
#define BeautyHighPassFilter_hpp

#include "mediapipe/render/core/Filter.hpp"
#include "mediapipe/render/core/Context.hpp"

namespace Opipe
{
    class BeautyHighPassFilter : public Opipe::Filter
    {
    public:
        static BeautyHighPassFilter *create(Opipe::Context *context);
        bool init(Opipe::Context *context);

    public:
        BeautyHighPassFilter(Opipe::Context *context);
        ~BeautyHighPassFilter(){};
    };
}

#endif