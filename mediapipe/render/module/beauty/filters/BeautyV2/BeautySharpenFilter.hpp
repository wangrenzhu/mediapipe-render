#ifndef BeautySharpenFilter_hpp
#define BeautySharpenFilter_hpp

#include "mediapipe/render/core/Filter.hpp"
#include "mediapipe/render/core/Context.hpp"

namespace Opipe
{
    class BeautySharpenFilter : public Opipe::Filter
    {
    public:
        static BeautySharpenFilter *create(Opipe::Context *context);
        bool init(Opipe::Context *context);

        virtual bool proceed(float frameTime = 0, bool bUpdateTargets = true) override;
        void update(float frameTime = 0) override;
        void setSharpness(float factor) {
            _sharpness = factor;
        }
    public:
        BeautySharpenFilter(Opipe::Context *context);
        ~BeautySharpenFilter(){};
    private:
        float _sharpness = 0.0;
        float _widthFactor = 0.0;
        float _heightFactor = 0.0;
    };
}

#endif