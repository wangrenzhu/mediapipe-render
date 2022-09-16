//
//  OlaPreStreamingIMP.cpp
//  OStream
//
//  Created by wangrenzhu2021 on 2021/11/26.
//  Copyright © 2021 alibaba. All rights reserved.
//

#include "OlaPreStreamingIMP.hpp"
#if defined(__APPLE__)
#include "CVFramebuffer.hpp"
#else
#include "OlaHardwarePreStreamingFilter.hpp"
#endif

#include "OlaPreStreamingFilter.hpp"

namespace Opipe {
    OlaPreStreamingIMP::~OlaPreStreamingIMP() {
        _context = 0;
        _dispatch = 0;
        if (_framebuffer) {
            delete _framebuffer;
            _framebuffer = 0;
        }
        if (_paddingTransformFilters.size() > 0) {

            for (auto filter : _paddingTransformFilters) {
                if (_textureSource != 0) {
                    _textureSource->removeTarget(filter);
                }
                filter->release();
            }
            _paddingTransformFilters.clear();

        }
        
        _textureSource = 0;
    }
    
    OlaPreStreamingIMP::OlaPreStreamingIMP(OpipeDispatch *_dispatch) {
        _context = _dispatch->currentContext();
        _dispatch = _dispatch;
    }

    void OlaPreStreamingIMP::reset() {
        _dispatch->runSync([&] {

            std::unique_lock<std::mutex> lock(_resetMutex);
            if (_paddingTransformFilters.size() > 0) {
                for (auto filter : _paddingTransformFilters) {
                    if (_textureSource != 0) {
                        _textureSource->removeTarget(filter);
                    }
                    delete filter;
                }
                _paddingTransformFilters.clear();
            }
            _textureSource = 0;
        });
    }

    void OlaPreStreamingIMP::loadStreamInfoSync(std::vector<OPreStreamInfo> streamInfos) {


        _dispatch->runSync([&] {
            _context->useAsCurrent();
            _streamInfos = streamInfos;
            std::unique_lock<std::mutex> lock(_resetMutex);

            for (const auto &info : _streamInfos) {
                if (info.type == OPTypeGPUbuffer || info.type == OPTypeCPUImageFrame) {
                    OlaPreStreamingFilter *paddingFilter = nullptr;
#if defined(__APPLE__)
                    paddingFilter = OlaPreStreamingFilter::create(_context, info.useGray);
#else
                    if (EGLAndroid::supportHardwareBuffer()) {
                        paddingFilter = OlaHardwarePreStreamingFilter::create(_context);
                        _usePBO = true;
                        paddingFilter->setNeedDownloadPixels(true); //暂时这么写后续关掉
                    }

                    if (paddingFilter == nullptr) {
                        return;
                    }
#endif
                    paddingFilter->setMaxSize(info.maxSize);
                    paddingFilter->setMinSize(info.minSize);
                    paddingFilter->setTargetWidth(info.width);
                    paddingFilter->setTargetHeight(info.height);
                    paddingFilter->setGray(info.useGray);
                    paddingFilter->setNeedPadding(info.needPadding);
                    paddingFilter->setNeedFixedPadding(info.needFixedPadding);
                    paddingFilter->setFlowFixedPadding(info.flowFixedPadding);
                    paddingFilter->setScaleOfValue(info.scaleOf);
                    paddingFilter->setStreamName(info.name);
                    paddingFilter->setPaddingColor((float) info.padding_color.r / 255.0,
                                                   (float) info.padding_color.g / 255.0,
                                                   (float) info.padding_color.b / 255.0);
                    paddingFilter->setUseGPU(info.type == OPTypeGPUbuffer);
                    _paddingTransformFilters.emplace_back(paddingFilter);
                    if (_textureSource && _textureSource) {
                        //如果有外部渲染器 统一渲染管线
                        _textureSource->addTarget(paddingFilter);
                    }
                }
            }
        });
    }
    
    void OlaPreStreamingIMP::preStream(OPreTextureInfo textureInfo, int64_t frameTime) {
        if (_isPaused) {
            return;
        }
        {
        if (!_textureSource) {
            _context->useAsCurrent();
            // BGRA 提交分流指令
            TextureAttributes inputAttr;
            inputAttr.type = textureInfo.textureAttributes.type;
            inputAttr.format = textureInfo.textureAttributes.format;
            inputAttr.magFilter = textureInfo.textureAttributes.magFilter;
            inputAttr.internalFormat = textureInfo.textureAttributes.internalFormat;
            inputAttr.minFilter = textureInfo.textureAttributes.minFilter;
            inputAttr.wrapS = textureInfo.textureAttributes.wrapS;
            inputAttr.wrapT = textureInfo.textureAttributes.wrapT;
            if (_framebuffer == nullptr ||
                (_framebuffer && _framebuffer->getTexture() != textureInfo.textureId)) {
                if (_framebuffer) {
                    delete _framebuffer;
                }
                //纹理id变了或者为空初始化
                _framebuffer = _context->getFramebufferCache()->
                fetchFramebufferUseTextureId(_context,
                                             textureInfo.width,
                                             textureInfo.height,
                                             textureInfo.textureId,
                                             true,
                                             inputAttr);
            }
        }
        
        for (const auto &filter : _paddingTransformFilters) {
            filter->setTargetRotationMode((RotationMode)textureInfo.deviceRotationMode);
            if (!_textureSource) {
                filter->setInputFramebuffer(_framebuffer);
                //有外部render的情况下 就不用触发了
                filter->update(frameTime); //实际触发渲染
            }
        }
        
        
        GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        glFlush();
        _dispatch->runAsync([&, frameTime, sync] {
            //这里取数据
            glWaitSync(sync, 0, 1000000);
            glDeleteSync(sync);
            std::vector<PreStreamFrameData> frameDatas;
            for (auto filter : _paddingTransformFilters) {
                //lock 保护读写同步问题
                PreStreamFrameData frameData;
                frameData.timeStamp = frameTime;
                if (filter->getUseGPU()) {
#if defined(__APPLE__)
                    //只有iOS 支持 GPUBuffer 安卓不支持
                    auto *cvFramebuffer = (CVFramebuffer *)filter->getFramebuffer();
                    if (cvFramebuffer == nullptr) {
                        continue;
                    }
                    
                    CVPixelBufferRef pixelBuffer = cvFramebuffer->renderTarget;
                    frameData.pixelBuffer = pixelBuffer;
                    frameData.stream_name = filter->getStreamName();
                    frameData.fixedWidthOffset = filter->fixedWidthOffset();
                    frameData.fixedHeightOffset = filter->fixedHeightOffset();
                    frameData.isGPUData = true;
                    if (@available(iOS 11.0, *)) {
                        frameData.surfaceId = IOSurfaceGetID(cvFramebuffer->renderIOSurface);
                    } else {
                        frameData.surfaceId = -1;
                    }
                    frameDatas.push_back(frameData);
#endif
                    // Android 暂时不支持 后续扩展
                } else {
                    filter->lock();
                    
                    frameData.stream_name = filter->getStreamName().c_str();
#if defined(__APPLE__)
                    auto *cvFramebuffer = (CVFramebuffer *)filter->getFramebuffer();
                    if (cvFramebuffer == nullptr) {
                        continue;
                    }
                    
                    CVPixelBufferRef pixelBuffer = cvFramebuffer->renderTarget;
                    frameData.pixelBuffer = pixelBuffer;
#endif

                    if (_usePBO) {
                        OlaImageFrameDes des;
                        char *data = nullptr;
                        //释放智能指针
                        OlaImageFrame *imageFrame = filter->cacheData.release();
                        if (imageFrame) {
                            imageFrame->releaseDataControl(&data, des);
                            //释放ImageFrame
                            delete imageFrame;
                        }
                        if (data) {
                            frameData.dataBuffers = data;
                            frameData.width = des.width;
                            frameData.height = des.height;
                            frameData.width_step = des.widthStep;
                            frameData.hasData = true;
                            frameData.needDelete = true;
                        }
                    } else {
                        auto *framebuffer = filter->getFramebuffer();
                        framebuffer->lockAddress();
                        int width_step = framebuffer->getBytesPerRow();
                        frameData.dataBuffers = (char *)framebuffer->frameBufferGetBaseAddress();
                        framebuffer->unlockAddress();
                        frameData.width = framebuffer->getWidth();
                        frameData.height = framebuffer->getHeight();
                        frameData.width_step = width_step;
                        frameData.hasData = true;
                        frameData.needDelete = false;
                        
                    }
                    frameData.fixedWidthOffset = filter->fixedWidthOffset();
                    frameData.fixedHeightOffset = filter->fixedHeightOffset();
                    frameDatas.push_back(frameData);

                }
            }
            
            if (frameDatas.size() > 0) {
                if (this->_callbackHolder && this->_callbackHolder->func) {
                    this->_callbackHolder->func(frameTime, frameDatas, this->_callbackHolder->other);
                }
            }
            for (auto filter : _paddingTransformFilters) {
                filter->unlock();
            }
        }, Context::IOContext);
        }
    }
    
    int OlaPreStreamingIMP::setCallBackHandler(const PreStreamDataCallback &func,
                                                       void *other) {
        if (this->_callbackHolder != 0) {
            this->_callbackHolder = nullptr; //释放掉
        }
        this->_callbackHolder = std::make_unique<PreStreamCallbackHolder>();
        this->_callbackHolder->func = func;
        this->_callbackHolder->other = other;
        return 1;
    }
    
    void OlaPreStreamingIMP::attachTextureSource(Source *textureSource) {
        _textureSource = textureSource;
        if (_paddingTransformFilters.size() > 0 && _textureSource) {
            for (const auto &filter : _paddingTransformFilters) {
                if(!_textureSource->hasTarget(filter)) {
                    _textureSource->addTarget(filter);
                }
            }
            delete _framebuffer;
        }
    }
    
    void OlaPreStreamingIMP::detachTextureSource(Source *textureSource) {

        if (_paddingTransformFilters.size() > 0 && _textureSource) {
            for (const auto &filter : _paddingTransformFilters) {
                _textureSource->removeTarget(filter);
            }
        }
        _textureSource = 0;
    }
    
    int OlaPreStreamingIMP::suspend() {
        for (auto filter : _paddingTransformFilters) {
            filter->setEnable(false);
        }
        _isPaused = true;
        return 1;
    }
    
    int OlaPreStreamingIMP::resume() {
        for (auto filter : _paddingTransformFilters) {
            filter->setEnable(true);
        }
        _isPaused = false;
        return 1;
    }
}
