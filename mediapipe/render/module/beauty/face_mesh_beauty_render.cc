#include "face_mesh_beauty_render.h"
#include "mediapipe/render/core/CVFramebuffer.hpp"
#if defined(__APPLE__)
#import <Foundation/Foundation.h>
#endif

#include "mediapipe/framework/port/logging.h"

namespace Opipe
{
    FaceMeshBeautyRender::FaceMeshBeautyRender(Context *context, OMat lutMat)
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
        
#else 
        _lutImage = SourceImage::create(context, lutMat.width, lutMat.height, lutMat.data);

#endif
        _olaBeautyFilter->setLUTImage(_lutImage);
    }

    FaceMeshBeautyRender::FaceMeshBeautyRender(Context *context, OMat lutMat, OMat grayMat)
    {
        _context = context;
        _olaBeautyFilterV2 = OlaBeautyFilterV2::create(context);
        _isRendering = false;
        
        _outputFilter = OlaShareTextureFilter::create(context);
        _olaBeautyFilterV2->addTarget(_outputFilter);
#if defined(__APPLE__)
        
        NSBundle *bundle = [NSBundle bundleForClass:NSClassFromString(@"OlaFaceUnity")];
        NSURL *lutURL = [bundle URLForResource:@"skinLookup" withExtension:@"png"];
        _lutImage = SourceImage::create(context, lutURL);
        NSURL *grayURL = [bundle URLForResource:@"skinGray" withExtension:@"png"];
        _grayImage = SourceImage::create(context, grayURL);
        
#else 
        _lutImage = SourceImage::create(context, lutMat.width, lutMat.height, lutMat.data);
        _grayImage = SourceImage::create(context, grayMat.width, grayMat.height, grayMat.data);

#endif
        _olaBeautyFilterV2->setLUTImage(_lutImage);
        _olaBeautyFilterV2->setGrayImage(_grayImage);
    }

    FaceMeshBeautyRender::~FaceMeshBeautyRender()
    {
        if (_olaBeautyFilterV2)
        {
            _olaBeautyFilterV2->removeAllTargets();
            _olaBeautyFilterV2->release();
            _olaBeautyFilterV2 = nullptr;
        }        
   
        if (_olaBeautyFilter)
        {
            _olaBeautyFilter->removeAllTargets();
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
        
        if (_olaBeautyFilter) {
            _olaBeautyFilter->setInputFramebuffer(_inputFramebuffer, NoRotation, 0, true);
            _olaBeautyFilter->update(inputTexture.frameTime);
        } else if (_olaBeautyFilterV2) {
            _olaBeautyFilterV2->setInputFramebuffer(_inputFramebuffer, NoRotation, 0, true);
            _olaBeautyFilterV2->update(inputTexture.frameTime);
        }
        _inputFramebuffer->unlock();
    }

    TextureInfo FaceMeshBeautyRender::outputRenderTexture(TextureInfo inputTexture)
    {
        if (_outputFilter == nullptr) {
            LOG(ERROR) << "###### FaceMeshModuleIMP _outputFilter null";
            return inputTexture;
        }
        LOG(INFO) << "###### FaceMeshModuleIMP _outputFilter not null" << _outputFilter;
        TextureInfo outputTexture;
        outputTexture.frameTime = inputTexture.frameTime;
        auto *outputFramebuffer = _outputFilter->getFramebuffer();
        if (outputFramebuffer) {
            LOG(INFO) << "###### FaceMeshModuleIMP _outputFilter have outputbuffer";
            outputTexture.width = outputFramebuffer->getWidth();
            outputTexture.height = outputFramebuffer->getHeight();
            outputTexture.textureId = outputFramebuffer->getTexture();
            #if defined(__APPLE__)
            auto *cvFramebuffer = dynamic_cast<CVFramebuffer *>(outputFramebuffer);
            IOSurfaceRef surface = cvFramebuffer->renderIOSurface;
            outputTexture.ioSurfaceId = IOSurfaceGetID(surface);
            #endif
            LOG(INFO) << "###### FaceMeshModuleIMP _outputFilter outputbuffer:" << outputTexture.textureId 
            << " width:" << outputTexture.width;
        } else {
            LOG(INFO) << "###### FaceMeshModuleIMP _outputFilter null outputbuffer";
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
 
        } else if (_olaBeautyFilterV2) {
            _olaBeautyFilterV2->setProperty("face", facePoints);
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
        } else if (_olaBeautyFilterV2) {
            _olaBeautyFilterV2->setProperty("skin", smoothing);
        }
    }

    void FaceMeshBeautyRender::setWhitening(float whitening)
    {
        _whitening = whitening;
        if (_olaBeautyFilter)
        {
            _olaBeautyFilter->setProperty("whiten", whitening);
        } else if (_olaBeautyFilterV2) {
            _olaBeautyFilterV2->setProperty("whiten", whitening);
        }
    }

    void FaceMeshBeautyRender::setNoseFactor(float noseFactor) {
        _noseFactor = noseFactor;
        if (_olaBeautyFilter) {
            _olaBeautyFilter->setProperty("nose", noseFactor);
        } else if (_olaBeautyFilterV2) {
            _olaBeautyFilterV2->setProperty("nose", noseFactor);
        }
    }
    
    void FaceMeshBeautyRender::setFaceSlim(float slimFactor) {
        _faceFactor = slimFactor;
        if (_olaBeautyFilter) {
            _olaBeautyFilter->setProperty("slim", slimFactor);
        } else if (_olaBeautyFilterV2) {
            _olaBeautyFilterV2->setProperty("slim", slimFactor);
        }
    }

    void FaceMeshBeautyRender::setEye(float eyeFactor) {
        _eyeFactor = eyeFactor;
        if (_olaBeautyFilter) {
            _olaBeautyFilter->setProperty("eye", eyeFactor);
        } else if (_olaBeautyFilterV2) {
            _olaBeautyFilterV2->setProperty("eye", eyeFactor);
        }
    }
    
    void FaceMeshBeautyRender::setInputSource(Source *source) {
        FilterGroup *filterGroup = nullptr;

        if (_olaBeautyFilter) {
            filterGroup = _olaBeautyFilter;
        } else if (_olaBeautyFilterV2) {
            filterGroup = _olaBeautyFilterV2;
        }

        source->addTarget(filterGroup);
        _source = source;
        _source->retain();
    }
    
    void FaceMeshBeautyRender::setSharpness(float sharpnessFactor) {
        if (_olaBeautyFilterV2) {
            _sharpnessFactor = sharpnessFactor;
            _olaBeautyFilterV2->setProperty("sharpness", sharpnessFactor);
        }
    }

}
