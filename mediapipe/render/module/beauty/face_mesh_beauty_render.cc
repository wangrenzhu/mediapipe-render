#include "face_mesh_beauty_render.h"
#include "mediapipe/render/core/CVFramebuffer.hpp"
#if defined(__APPLE__)
#import <Foundation/Foundation.h>
#endif

namespace Opipe
{
    FaceMeshBeautyRender::FaceMeshBeautyRender(Context *context)
    {
        _context = context;
        _olaBeautyFilter = OlaBeautyFilter::create(context);
        _isRendering = false;
        
        _outputFilter = OlaShareTextureFilter::create(context);
        _olaBeautyFilter->addTarget(_outputFilter);
        
#if defined(__APPLE__)
        
        NSBundle *bundle = [NSBundle bundleForClass:NSClassFromString(@"OlaFaceUnity")];
        
        NSURL *lutURL =  [bundle URLForResource:@"whiten" withExtension:@"png"];
        _lutImage = SourceImage::create(context, lutURL);
        
#endif
        
        _olaBeautyFilter->setLUTImage(_lutImage);
    }

    FaceMeshBeautyRender::~FaceMeshBeautyRender()
    {
        _olaBeautyFilter->removeAllTargets();
   
        if (_olaBeautyFilter)
        {
            _olaBeautyFilter->release();
            _olaBeautyFilter = nullptr;
        }

        if (_outputFilter)
        {
            _outputFilter->release();
            _outputFilter = nullptr;
        }
        
        if (_lutImage)
        {
            auto *framebuffer = _lutImage->getFramebuffer();
            delete framebuffer;
            _lutImage->release();
            _lutImage = nullptr;
        }
        
        if (_inputFramebuffer) {
            delete _inputFramebuffer;
            _inputFramebuffer = nullptr;
        }

        if (_source)
        {
            _source->removeAllTargets();
            _source->release();
            _source = nullptr;
        }
        
        
        _context->getFramebufferCache()->purge();
    }

    void FaceMeshBeautyRender::suspend()
    {
        _isRendering = false;
    }

    void FaceMeshBeautyRender::resume()
    {
        _isRendering = true;
    }

    void FaceMeshBeautyRender::renderTexture(TextureInfo inputTexture)
    {   
        if (!_isRendering || _source) {
            return;
        }
        
        if (!_inputFramebuffer)
        {
            _inputFramebuffer = new Framebuffer(_context, inputTexture.width, inputTexture.height,
                                                Framebuffer::defaultTextureAttribures,
                                                inputTexture.textureId);
        }
        else if (_inputFramebuffer->getWidth() != inputTexture.width || _inputFramebuffer->getHeight() != inputTexture.height)
        {
            _inputFramebuffer->unlock();
            delete _inputFramebuffer;
            _inputFramebuffer = nullptr;
            _inputFramebuffer = new Framebuffer(_context, inputTexture.width, inputTexture.height,
                                                Framebuffer::defaultTextureAttribures,
                                                inputTexture.textureId);
        }
        _inputFramebuffer->lock();
        _olaBeautyFilter->setInputFramebuffer(_inputFramebuffer, NoRotation, 0, true);
        _olaBeautyFilter->update(inputTexture.frameTime);
        _inputFramebuffer->unlock();
    }

    TextureInfo FaceMeshBeautyRender::outputRenderTexture(TextureInfo inputTexture)
    {
        if (_outputFilter == nullptr) {
            return inputTexture;
        }
        
        TextureInfo outputTexture;
        outputTexture.frameTime = inputTexture.frameTime;
        auto *outputFramebuffer = _outputFilter->getFramebuffer();
        if (outputFramebuffer) {
            outputTexture.width = outputFramebuffer->getWidth();
            outputTexture.height = outputFramebuffer->getHeight();
            outputTexture.textureId = outputFramebuffer->getTexture();
            #if defined(__APPLE__)
            auto *cvFramebuffer = dynamic_cast<CVFramebuffer *>(outputFramebuffer);
            IOSurfaceRef surface = cvFramebuffer->renderIOSurface;
            outputTexture.ioSurfaceId = IOSurfaceGetID(surface);
            #endif
        } else {
            outputTexture.width = inputTexture.width;
            outputTexture.height = inputTexture.height;
            outputTexture.textureId = inputTexture.textureId;
            outputTexture.ioSurfaceId = inputTexture.ioSurfaceId;
        }
        return outputTexture;
    }

    void FaceMeshBeautyRender::setFacePoints(std::vector<Vec2> facePoints) {
        if (_olaBeautyFilter) {
            _olaBeautyFilter->setProperty("face", facePoints);

        }
    }

    void FaceMeshBeautyRender::setUseSegmentation(bool useSegmentation) {
        if (_olaBeautyFilter) {
            _useSegmentation = useSegmentation;
            _olaBeautyFilter->setUseSegmentation(_useSegmentation);
        }
    }

    void FaceMeshBeautyRender::setSegmentationBackground(SourceImage *background) {
        if (_olaBeautyFilter) {
            _olaBeautyFilter->setSegmentationBackground(background);
        }
    }

    void FaceMeshBeautyRender::setSegmentationMask(Framebuffer *maskbuffer) {
        if (_olaBeautyFilter) {
            // 有个异步问题 需要外部处理WaitOnGPU
            _olaBeautyFilter->setSegmentationMask(maskbuffer);
        }
    }

    float FaceMeshBeautyRender::getSmoothing()
    {
        return _smoothing;
    }

    float FaceMeshBeautyRender::getWhitening()
    {
        return _whitening;
    }

    void FaceMeshBeautyRender::setSmoothing(float smoothing)
    {
        _smoothing = smoothing;
        if (_olaBeautyFilter)
        {
            _olaBeautyFilter->setProperty("skin", smoothing);
        }
    }

    void FaceMeshBeautyRender::setWhitening(float whitening)
    {
        _whitening = whitening;
        if (_olaBeautyFilter)
        {
            _olaBeautyFilter->setProperty("whiten", whitening);
        }
    }

    void FaceMeshBeautyRender::setNoseFactor(float noseFactor) {
        _noseFactor = noseFactor;
        if (_olaBeautyFilter) {
            _olaBeautyFilter->setProperty("nose", noseFactor);
        }
    }
    
    void FaceMeshBeautyRender::setFaceSlim(float slimFactor) {
        _faceFactor = slimFactor;
        if (_olaBeautyFilter) {
            _olaBeautyFilter->setProperty("slim", slimFactor);
        }
    }

    void FaceMeshBeautyRender::setEye(float eyeFactor) {
        _eyeFactor = eyeFactor;
        if (_olaBeautyFilter) {
            _olaBeautyFilter->setProperty("eye", eyeFactor);
        }
    }
    
    void FaceMeshBeautyRender::setInputSource(Source *source) {
        source->addTarget(_olaBeautyFilter);
        _source = source;
        _source->retain();
    }

}
