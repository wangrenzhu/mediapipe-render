#include "OlaBeautyFilterV2.hpp"
#include "mediapipe/render/core/math/vec2.hpp"
#include "mediapipe/framework/port/logging.h"

namespace Opipe {
    OlaBeautyFilterV2::OlaBeautyFilterV2(Context *context) : FilterGroup(context)
    {

    }

    OlaBeautyFilterV2::~OlaBeautyFilterV2()
    {
        if (_lutImage) {
            _lutImage->release();
            _lutImage = nullptr;
        }

        if (_grayImage)
        {
            _grayImage->release();
            _grayImage = nullptr;
        }
        
        if (_beautyBlurFilter)
        {
            _beautyBlurFilter->release();
            _beautyBlurFilter = nullptr;
        }

        if (_highPassBlurFilter)
        {
            _highPassBlurFilter->release();
            _highPassBlurFilter = nullptr;
        }

        if (_highPassFilter)
        {
            _highPassFilter->release();
            _highPassFilter = nullptr;
        }

        if (_complexionFilter)
        {
            _complexionFilter->release();
            _complexionFilter = nullptr;
        }

        if (_adjustFilter)
        {
            _adjustFilter->release();
            _adjustFilter = nullptr;
        }
        
        if (_faceDistortFilter) {
            _faceDistortFilter->release();
            _faceDistortFilter = nullptr;
        }

        if (_beautySharpenFilter) {
            _beautySharpenFilter->release();
            _beautySharpenFilter = nullptr;
        }
        
    }

    OlaBeautyFilterV2 *OlaBeautyFilterV2::create(Context *context)
    {
        OlaBeautyFilterV2 *ret = new (std::nothrow)OlaBeautyFilterV2(context);
        if (ret && ret->init(context)) {
            return ret;
        } else {
            delete ret;
            return nullptr;
        }
    }

    bool OlaBeautyFilterV2::init(Context *context) {
        if (!FilterGroup::init(context)) {
            return false;
        }

        Log("OlaBeautyFilterV2", "init");
        _complexionFilter = BeautyComplexion::create(context);
        Log("OlaBeautyFilterV2", "init BeautyComplexion");
        _beautyBlurFilter = BeautyGaussBlurFilter::create(context);
        Log("OlaBeautyFilterV2", "init _beautyBlurFilter");
        _beautyBlurFilter->setFramebufferScale(0.5);

        _highPassFilter = BeautyHighPassFilter::create(context);

        _highPassBlurFilter = BeautyGaussBlurFilter::create(context);

        _highPassBlurFilter->setFramebufferScale(0.5);

        _adjustFilter = BeautyAdjustFilter::create(context);

        _faceDistortFilter = FaceDistortionFilter::create(context);

        _beautySharpenFilter = BeautySharpenFilter::create(context);
        addFilter(_complexionFilter);

        _complexionFilter->addTarget(_highPassFilter)->addTarget(_highPassBlurFilter);
        _complexionFilter->addTarget(_beautyBlurFilter)->addTarget(_highPassFilter, 1);
        
        _complexionFilter->addTarget(_beautySharpenFilter)->addTarget(_adjustFilter);
        _beautyBlurFilter->addTarget(_adjustFilter, 1);
        _highPassBlurFilter->addTarget(_adjustFilter, 2);
        
        _adjustFilter->addTarget(_faceDistortFilter);

        setTerminalFilter(_faceDistortFilter);
        std::vector<Vec2> defaultFace;
        registerProperty("face", defaultFace, "人脸点", [this](std::vector<Vec2> facePoints) {
            if (facePoints.size() == 0) {
                _faceDistortFilter->setEnable(false);
                return;
            }
            _faceDistortFilter->setEnable(true);
            _faceDistortFilter->setFacePoints(facePoints);
        });

        registerProperty("eye", 1.0f, "大眼 0.0 - 1.0",
                         [this](float eye) {
            _faceDistortFilter->setEye(eye);
        });

        registerProperty("slim", 1.0f, "瘦脸 0.0 - 1.0",
                         [this](float slim) {
            _faceDistortFilter->setSlim(slim);
        });
        
        registerProperty("nose", 1.0f, "瘦鼻 0.0 - 1.0",
                         [this](float nose) {
            _faceDistortFilter->setNose(nose);
        });
        
        registerProperty("skin", 1.0f, "磨皮 0.0 - 1.0",
                         [this](float skin) {
            _adjustFilter->setIndensity(skin);
        });
        
        registerProperty("sharpness", 0.0f, "锐化 0.0 - 1.0",
                         [this](float sharpness) {
            _beautySharpenFilter->setSharpness(sharpness);
        });

        registerProperty("whiten", 1.0f, "美白 0.0 - 1.0",
                         [this](float whiten) {
            _complexionFilter->setComplexionLevel(whiten);
        });

        return true;
        
    }

    bool OlaBeautyFilterV2::proceed(float frameTime, bool bUpdateTargets) {
        return FilterGroup::proceed(frameTime, bUpdateTargets);
    }

    void OlaBeautyFilterV2::update(float frameTime) {
        FilterGroup::update(frameTime);
    }

    void OlaBeautyFilterV2::setLUTImage(SourceImage *lutImage) {
        _lutImage = lutImage;
        _lutImage->retain();
        if (_complexionFilter) {
            auto *framebuffer = lutImage->getFramebuffer();
            _complexionFilter->setInputFramebuffer(framebuffer, NoRotation, 2, true);
        }
    }

    void OlaBeautyFilterV2::setGrayImage(SourceImage *grayImage) {
        _grayImage = grayImage;
        _grayImage->retain();
        if (_complexionFilter) {
            auto *framebuffer = grayImage->getFramebuffer();
            _complexionFilter->setInputFramebuffer(framebuffer, NoRotation, 1, true);
        }
    }

    void OlaBeautyFilterV2::setInputFramebuffer(Framebuffer *framebuffer,
                                                        RotationMode rotationMode,
                                                        int texIdx,
                                                        bool ignoreForPrepared) {
        for (auto& filter : _filters) {
            filter->setInputFramebuffer(framebuffer, rotationMode, texIdx, ignoreForPrepared);
        }
    }
}
