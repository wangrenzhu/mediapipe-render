#include "OlaCameraSource.hpp"
#include "Context.hpp"
#if defined(__APPLE__)
#import <OpenGLES/EAGLIOSurface.h>
#endif

using namespace Opipe;

namespace Opipe
{

    OlaCameraSource::OlaCameraSource(Context *context, SourceType sourceType) : SourceCamera(context)
    {
        _sourceType = sourceType;
        _lastIOSurface = -1;
        
        switch (_sourceType)
        {
            case SourceType_RGBA:
                _yuvTexture = nullptr;
                break;
            case SourceType_YUV420SP:
                _yuvTexture = OlaYUVTexture::create(context);
                break;
            case SourceType_YUV420P:
                _yuvTexture = OlaYUVTexture420P::create(context);
                break;
            default:
                break;
        }
        if (_yuvTexture) {
            _faceTexture = OlaShareTextureFilter::create(context);
            _renderTexture = OlaShareTextureFilter::create(context);
            _segmentTexture = OlaShareTextureFilter::create(context);
            _faceTexture->setTargetSize(Vector2(192.0, 192.0));
            _segmentTexture->setTargetSize(Vector2(256.0, 256.0));
            addTarget(_yuvTexture);
            _yuvTexture->addTarget(_faceTexture);
            _yuvTexture->addTarget(_renderTexture);
            _yuvTexture->addTarget(_segmentTexture);
        } else {
            _renderTexture = OlaShareTextureFilter::create(context);
            _faceTexture = OlaShareTextureFilter::create(context);
            _segmentTexture = OlaShareTextureFilter::create(context);
            _faceTexture->setTargetSize(Vector2(192.0, 192.0));
            _segmentTexture->setTargetSize(Vector2(256.0, 256.0));
            addTarget(_faceTexture);
            addTarget(_renderTexture);
            addTarget(_segmentTexture);
        }
    }

    OlaCameraSource::~OlaCameraSource()
    {
        if (_yuvTexture)
        {
            _yuvTexture->removeAllTargets();
            _yuvTexture->release();
            _yuvTexture = nullptr;
        }
        
        if (_faceTexture) {
            _faceTexture->release();
            _faceTexture = nullptr;
        }
        
        if (_renderTexture) {
            _renderTexture->release();
            _renderTexture = nullptr;
        }
        
        if (_segmentTexture) {
            _segmentTexture->release();
            _segmentTexture = nullptr;
        }
    }

    void OlaCameraSource::setFrameData(int width,
                                       int height,
                                       const void *pixels,
                                       GLenum type,
                                       GLuint texture,
                                       RotationMode outputRotation,
                                       SourceType sourceType,
                                       const void *upixels,
                                       const void *vpixels,
                                       bool keep_white)
    {
        if (_sourceType != sourceType)
        {
            _sourceType = sourceType;
            if (_yuvTexture)
            {
                _yuvTexture->removeAllTargets();
                _yuvTexture->release();
                _yuvTexture = nullptr;
            }
            
            removeAllTargets();
            
            switch (_sourceType)
            {
                case SourceType_RGBA:
                    _yuvTexture = nullptr;
                    break;
                case SourceType_YUV420SP:
                    _yuvTexture = OlaYUVTexture::create(_context);
                    break;
                case SourceType_YUV420P:
                    _yuvTexture = OlaYUVTexture420P::create(_context);
                    break;
                default:
                    break;
            }
            if (_yuvTexture) {
                addTarget(_yuvTexture);
            }
        }
        
        SourceCamera::setFrameData(width, height, pixels, type, texture,
                                   outputRotation, sourceType,
                                   upixels, vpixels, keep_white);
    }

    OlaCameraSource* OlaCameraSource::create(Context *context)
    {
        return new OlaCameraSource(context);
    }

    Source* OlaCameraSource::addTarget(Target *target)
    {
        if (_yuvTexture && target != _yuvTexture)
        {
            return _yuvTexture->addTarget(target);
        }
        return SourceCamera::addTarget(target);
    }

    #if defined(__APPLE__)
    void OlaCameraSource::setIORenderTexture(IOSurfaceID surfaceID,
                                             GLuint texture,
                                             int width,
                                             int height,
                                             RotationMode outputRotation,
                                             SourceType sourceType,
                                             const TextureAttributes textureAttributes)
    {
        // iOS 版不支持切换格式 要么RGBA 要么YUV420F
        _sourceType = sourceType;
        if (sourceType == SourceType_RGBA) {
            SourceCamera::setIORenderTexture(surfaceID, texture, width, height,
                                             outputRotation, sourceType, textureAttributes);
        } else {
            if (surfaceID != _lastIOSurface) {
                // surfaceID 变了需要重新创建Framebuffer
                _bindIOSurfaceToTexture(surfaceID);
                _lastIOSurface = surfaceID;
            }
            setFramebuffer(_framebuffer, outputRotation);
        }
    }

    void OlaCameraSource::_bindIOSurfaceToTexture(int iosurface, RotationMode outputRotation)
    {
        IOSurfaceRef surface = IOSurfaceLookup(iosurface);
        int width = (int)IOSurfaceGetWidth(surface);
        int height = (int)IOSurfaceGetHeight(surface);
        if (surface)
        {
            if (_UVFrameBuffer == nullptr) {
                _UVFrameBuffer = _context->getFramebufferCache()->
                fetchFramebuffer(_context, width * 0.5, height * 0.5, true);
            }
            EAGLContext *eglContext = _context->getEglContext();
            if (_UVFrameBuffer) {
                _UVFrameBuffer->active();
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                                GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                
                BOOL rs = [eglContext texImageIOSurface:surface target:GL_TEXTURE_2D internalFormat:GL_LUMINANCE_ALPHA
                                                  width:width * 0.5 height:height * 0.5 format:GL_LUMINANCE_ALPHA type:GL_UNSIGNED_BYTE plane:1];
                if (rs) {
                    Log("Opipe", "IOSurface 绑定UV Texture 成功");
                }
            }
            
            this->setFramebuffer(nullptr);
            Framebuffer* framebuffer = _context->getFramebufferCache()->fetchFramebuffer(_context, width, height, true);
            
            this->setFramebuffer(framebuffer, outputRotation);
            
            _framebuffer->active();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                            GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            BOOL rs = [eglContext texImageIOSurface:surface target:GL_TEXTURE_2D internalFormat:GL_LUMINANCE
                                              width:width height:height format:GL_LUMINANCE type:GL_UNSIGNED_BYTE plane:0];
            if (rs) {
                Log("Opipe", "IOSurface 绑定Y Texture 成功");
            }
            
        }
    }

    Framebuffer* OlaCameraSource::getSegmentationFramebuffer() {
        if (_segmentTexture && _segmentTexture->getFramebuffer()) {
            return _segmentTexture->getFramebuffer();
        } else {
            return nullptr;
        }
    }

    Framebuffer* OlaCameraSource::getFaceFramebuffer() {
        if (_faceTexture && _faceTexture->getFramebuffer()) {
            return _faceTexture->getFramebuffer();
        } else {
            return nullptr;
        }
    }

    Framebuffer* OlaCameraSource::getRenderFramebuffer() {
        if (_sourceType == SourceType_RGBA) {
            if (_framebuffer) {
                return _framebuffer;
            } else {
                return nullptr;
            }
        } else {
            if (_renderTexture && _renderTexture->getFramebuffer() && !_renderTexture->getFramebuffer()->isDealloc) {
                return _renderTexture->getFramebuffer();
            }
        }
        return nullptr;
    }
    #endif
}
