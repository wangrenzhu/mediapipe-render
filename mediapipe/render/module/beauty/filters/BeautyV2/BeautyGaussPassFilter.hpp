#ifndef BeautyGaussPassFilter_hpp
#define BeautyGaussPassFilter_hpp

#include "mediapipe/render/core/Filter.hpp"
#include "mediapipe/render/core/Context.hpp"

namespace Opipe
{
    class BeautyGaussPassFilter : public Opipe::Filter
    {
    public:
        enum Type {HORIZONTAL, VERTICAL};
        static BeautyGaussPassFilter *create(Opipe::Context *context, Type type = HORIZONTAL);
        bool init(Opipe::Context *context);

        virtual bool proceed(float frameTime = 0, bool bUpdateTargets = true) override;
        virtual void update(float frameTime) override;
        void setBlurSize(float blurSize) {
            _blurSize = blurSize;
        }

        void setTexelOffsetSize(float width, float height) {
            _texelWidth = width;
            _texelHeight = height;
            if (_texelWidth != 0.0) {
                _texelWidthOffset = _blurSize / _texelWidth;
            } else {
                _texelWidthOffset = 0.0;
            }

            if (_texelHeight != 0.0) {
                _texelHeightOffset = _blurSize / _texelHeight;
            } else {
                _texelHeightOffset = 0.0;
            }
        }

    public:
        BeautyGaussPassFilter(Opipe::Context *context, Type type = HORIZONTAL);
        ~BeautyGaussPassFilter(){};
    private:
        float _texelWidth = 0.0;
        float _texelHeight = 0.0;
        float _blurSize = 1.0;
        float _texelWidthOffset = 0.0;
        float _texelHeightOffset = 0.0;
        Type _type;
        GLuint _texelWidthOffsetLocation = -1;
        GLuint _texelHeightOffsetLocation = -1;

    };
}

#endif
