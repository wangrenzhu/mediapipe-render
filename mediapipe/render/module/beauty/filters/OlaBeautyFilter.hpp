#include "mediapipe/render/core/Filter.hpp"
#include "mediapipe/render/core/FilterGroup.hpp"
#include "mediapipe/render/core/BilateralFilter.hpp"
#include "mediapipe/render/core/AlphaBlendFilter.hpp"
#include "mediapipe/render/core/LUTFilter.hpp"
#include "mediapipe/render/core/OlaShareTextureFilter.hpp"
#include "mediapipe/render/core/SourceImage.hpp"
#include "mediapipe/render/module/beauty/filters/BeautyV2/FaceDistortionFilter.hpp"
#include "BilateralAdjustFilter.hpp"
#include "UnSharpMaskFilter.hpp"
#include "OlaSegmentOutlineFilter.hpp"
#if defined(__APPLE__)
#include "mediapipe/render/core/CVFramebuffer.hpp"
#endif

namespace Opipe
{
    class OlaBeautyFilter : public FilterGroup
    {

    public:
        static OlaBeautyFilter *create(Context *context);

        bool init(Context *context);

        bool proceed(float frameTime = 0, bool bUpdateTargets = true) override;

        void update(float frameTime = 0) override;

        virtual void setInputFramebuffer(Framebuffer *framebuffer,
                                         RotationMode rotationMode =
                                             RotationMode::NoRotation,
                                         int texIdx = 0,
                                         bool ignoreForPrepared = false) override;

        void setLUTImage(SourceImage *image);

        OlaBeautyFilter(Context *context);

        virtual ~OlaBeautyFilter();
        
#if defined(__APPLE__)
        void updateSegmentMarkIOSurfaceId(IOSurfaceID surfaceId, int width, int height) {
            if (_segmentMask == nullptr) {
                _segmentMask = new CVFramebuffer(_context, width, height, surfaceId);
                if (_segmentOutlineFilter) {
                    _segmentOutlineFilter->setInputFramebuffer(_segmentMask, NoRotation, 1, true);
                }
            }
            
            IOSurfaceID sId = ((CVFramebuffer *)_segmentMask)->_ioSurfaceId;
            
            if (sId != surfaceId) {
                Log("BeautyFilter", " surfaceIdChanged: old:%d new:%d", sId, surfaceId);
                if (_segmentOutlineFilter) {
                    _segmentOutlineFilter->setInputFramebuffer(nullptr, NoRotation, 1);
                }
                delete _segmentMask;
                _segmentMask = new CVFramebuffer(_context, width, height, surfaceId);
            }
            
            if (_segmentMask && _segmentOutlineFilter) {
                _segmentOutlineFilter->setInputFramebuffer(_segmentMask, NoRotation, 1, true);
            }
            
            
            
        }
#endif
        
        // 更新分割的纹理
        // @param segmentTex 分割纹理 
        void updateSegmentMarkTexture(GLuint maskTexture, int width, int height) {
            Log("updateSegmentMarkTexture", "updateSegmentMarkTexture:%d", maskTexture);
            RotationMode rotation;
            rotation = NoRotation;
            if (_segmentMask == nullptr) {
                _segmentMask = new Framebuffer(_context, width, height, 
                                               Framebuffer::defaultTextureAttribures, maskTexture);
                if (_segmentOutlineFilter) {
                    _segmentOutlineFilter->setInputFramebuffer(_segmentMask, rotation, 1, true);
                }
            }
            GLuint texture = _segmentMask->getTexture();
            if (texture != maskTexture && _segmentOutlineFilter) {
                if (_segmentOutlineFilter) {
                    _segmentOutlineFilter->setInputFramebuffer(nullptr, rotation, 1);
                }
                delete _segmentMask;
                _segmentMask = new Framebuffer(_context, width, height,
                                               Framebuffer::defaultTextureAttribures, maskTexture);
            }
            
            if (_segmentMask && _segmentOutlineFilter) {
                _segmentOutlineFilter->setInputFramebuffer(_segmentMask, rotation, 1, true);
            }
        }
        
        void setSegmentEnable(bool enable);

        void setFaceLandmarkEnable(bool enable) {
            _faceDistortFilter->setEnable(enable);
        }

    private:
        BilateralFilter *_bilateralFilter = 0;
        AlphaBlendFilter *_alphaBlendFilter = 0;
        LUTFilter *_lutFilter = 0;
        BilateralAdjustFilter *_bilateralAdjustFilter = 0;
        UnSharpMaskFilter *_unSharpMaskFilter = 0;
        FaceDistortionFilter *_faceDistortFilter = 0;
        FilterGroup *_lookUpGroupFilter = 0;
        SourceImage *_lutImage = 0;
        OlaSegmentOutlineFilter *_segmentOutlineFilter = 0;
        Framebuffer *_segmentMask = 0;
        bool _useSegment = false;

        OlaShareTextureFilter *_testFilter = 0;
    };
}
