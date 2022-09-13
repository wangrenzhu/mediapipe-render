#ifndef BeautyComplexion_hpp
#define BeautyComplexion_hpp

#include "mediapipe/render/core/Filter.hpp"
#include "mediapipe/render/core/Context.hpp"

namespace Opipe
{
    class BeautyComplexion : public Opipe::Filter
    {
    public:
        static BeautyComplexion *create(Opipe::Context *context);
        bool init(Opipe::Context *context);

        virtual bool proceed(float frameTime = 0, bool bUpdateTargets = true) override;

    public:
        BeautyComplexion(Opipe::Context *context);
        ~BeautyComplexion(){};

        void setComplexionLevel(float level) {
            _alpha = level;
        }

    private:
        GLuint _levelRangeInvLocation = -1;
        GLuint _levelBlackLocation = -1;
        GLuint _alphaLocation = -1;

        float _levelRangeInv = 1.040816f;
        float _levelBlack = 0.01960784f;
        float _alpha = 1.0;
    };
}

#endif
