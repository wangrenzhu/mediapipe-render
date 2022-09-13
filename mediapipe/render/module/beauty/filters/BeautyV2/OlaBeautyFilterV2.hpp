#include "mediapipe/render/core/FilterGroup.hpp"
#include "mediapipe/render/core/SourceImage.hpp"
#include "FaceDistortionFilter.hpp"
#include "BeautyAdjustFilter.hpp"
#include "BeautyComplexion.hpp"
#include "BeautyGaussBlurFilter.hpp"
#include "BeautyHighPassFilter.hpp"
#include "BeautySharpenFilter.hpp"


namespace Opipe
{
    class OlaBeautyFilterV2 : public FilterGroup
    {

    public:
        static OlaBeautyFilterV2 *create(Context *context);

        bool init(Context *context);

        bool proceed(float frameTime = 0, bool bUpdateTargets = true) override;

        void update(float frameTime = 0) override;

        virtual void setInputFramebuffer(Framebuffer *framebuffer,
                                         RotationMode rotationMode =
                                             RotationMode::NoRotation,
                                         int texIdx = 0,
                                         bool ignoreForPrepared = false) override;
        // 美肤滤镜使用
        void setLUTImage(SourceImage *image);

        // 美肤滤镜使用
        void setGrayImage(SourceImage *grayImage);

        OlaBeautyFilterV2(Context *context);

        virtual ~OlaBeautyFilterV2();

    private:
        SourceImage *_lutImage = 0;
        SourceImage *_grayImage = 0;
        // 高斯模糊
        BeautyGaussBlurFilter *_beautyBlurFilter = 0;
        // 高通滤波做高斯模糊处理，保留边沿细节
        BeautyGaussBlurFilter *_highPassBlurFilter = 0;
        // 高通滤波
        BeautyHighPassFilter *_highPassFilter = 0;
        BeautyComplexion *_complexionFilter = 0;
        // 磨皮程度调节滤镜
        BeautyAdjustFilter *_adjustFilter = 0;
        FaceDistortionFilter *_faceDistortFilter = 0;
        BeautySharpenFilter *_beautySharpenFilter = 0;
    };
}
