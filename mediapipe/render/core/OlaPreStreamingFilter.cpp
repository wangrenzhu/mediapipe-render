//
//  OlaPreStreamingFilter.cpp
//  Quaramera
//
//  Created by wangrenzhu on 2021/4/16.
//  Copyright © 2021 alibaba. All rights reserved.
//

#include "OlaPreStreamingFilter.hpp"

namespace Opipe {

    const char kOlaPreStreamingFilterVertexShaderString[] =
    R"(#version 300 es
    layout(location = 0) in vec4 position;
    layout(location = 1) in vec4 texCoord;
    
    out vec2 vTexCoord;
    
    void main()
    {
    
        gl_Position = position;
        vTexCoord = texCoord.xy;
    })";
    
    const char kOlaPreStreamingFilterFragmentShaderString[] =
R"(#version 300 es
    
        precision highp float;
        in highp vec2 vTexCoord;
        uniform sampler2D colorMap;
        uniform vec2 paddingFactor;
        uniform float paddingColorR;
        uniform float paddingColorG;
        uniform float paddingColorB;
        uniform vec2 fixedOffset;
        layout(location = 0) out vec4 outColor;
        const highp vec3 W = vec3(0.2125, 0.7154, 0.0721);

        vec2 scalePosition(vec2 orginUv,vec2 scale){
            return(orginUv-.5)*scale+.5;
        }

        void main()
        {
            vec2 uv=vTexCoord;
            lowp vec4 textureColor;
            
            vec3 paddingColor=vec3(paddingColorR,paddingColorG,paddingColorB);
            
            if(fixedOffset.x+fixedOffset.y>0.0) {
                vec2 scale=1.-fixedOffset;
                vec2 uv2=uv-vec2(0.,fixedOffset.y);
                vec2 nUv=uv2/scale;
                
                textureColor=texture(colorMap,nUv);
                vec3 foreColor=paddingColor;
                float a=1.;
                a=step(uv.x,1.-fixedOffset.x);
                a*=(1.-step(uv.y,fixedOffset.y));
                
                foreColor=mix(paddingColor,textureColor.rgb,a);
                
                outColor=vec4(foreColor,1.);
            } else {
                vec2 scale=vec2(1./(1.-paddingFactor.x),1./(1.-paddingFactor.y));
                vec2 nUv=scalePosition(uv,scale);
                textureColor=texture(colorMap,nUv);
                
                vec3 foreColor=paddingColor;
                float a=1.;
                a=step(uv.x,1.-paddingFactor.x*.5)*(1.-step(uv.x,paddingFactor.x*.5));
                a*=(1.-step(uv.y,paddingFactor.y*.5))*(step(uv.y,1.-paddingFactor.y*.5));
                foreColor=mix(paddingColor,textureColor.rgb,a);
                
                outColor=vec4(foreColor,1.);
            }
            
        }
)";
    
    const char kOlaPreStreamingFilterGrayFragmentShaderString[] =
R"(#version 300 es
    
        precision highp float;
        in highp vec2 vTexCoord;
        uniform sampler2D colorMap;
        uniform vec2 paddingFactor;
        uniform float paddingColorR;
        uniform float paddingColorG;
        uniform float paddingColorB;
        uniform vec2 fixedOffset;
        layout(location = 0) out float outColor;
        const highp vec3 W = vec3(0.2125, 0.7154, 0.0721);
        void main()
        {
            vec2 uv = vTexCoord;
            lowp vec4 textureColor;
            textureColor = texture(colorMap, uv);
        
            float luminance = dot(textureColor.rgb, W);
            outColor = luminance;
        }
)";

    OlaPreStreamingFilter::OlaPreStreamingFilter(Context *context) : Filter(context),
                                                                           _paddingFactor(0.0, 0.0) {
                                                                               
    }

    OlaPreStreamingFilter *OlaPreStreamingFilter::create(Context *context, bool useGray) {
        auto *ret = new(std::nothrow) OlaPreStreamingFilter(context);
        ret->setGray(useGray);
        if (!ret || !ret->init(context)) {
            delete ret;
            ret = nullptr;
        }
        return ret;
    }

    bool OlaPreStreamingFilter::init(Context *context) {
        this->setEnable(true); //默认打开
        bool ret = false;
        if (_useGray) {
            ret = Filter::initWithShaderString(context,
                                                         std::string(kOlaPreStreamingFilterVertexShaderString),
                                                         std::string(kOlaPreStreamingFilterGrayFragmentShaderString));
            _format = (int)OlaImageFormat::GRAY8;
        } else {
            ret = Filter::initWithShaderString(context,
                                                         std::string(kOlaPreStreamingFilterVertexShaderString),
                                                         std::string(kOlaPreStreamingFilterFragmentShaderString));
            _format = (int)OlaImageFormat::SRGBA;
        }
        if (!ret) {
            return false;
        } else {

        }
        return true;
    }

    void OlaPreStreamingFilter::prepareFrameData(void *data, int width,
                                                    int height, int width_step) {
        if (_useGray) {
            _widthStep = width_step;
            cacheData = std::make_unique<OlaImageFrame>();
            if (cacheData) {
                cacheData->copyData((char *) data, width, height, _widthStep, OlaImageFormat::GRAY8);
            }
        } else {
            _widthStep = width_step;
            cacheData = std::make_unique<OlaImageFrame>();
            if (cacheData) {
                cacheData->copyData((char *) data, width, height, _widthStep, OlaImageFormat::SRGBA);
            }
        }
    }

    OlaPreStreamingFilter::~OlaPreStreamingFilter() {
        _framebuffer = 0;
        cacheData = nullptr;
    }

    void OlaPreStreamingFilter::update(float frameTime) {
        if (isLocked) {
            //锁了不写数据
            return;
        }
        
        if (!_enable) {
            //没有激活 就不要处理
            return;
        }

        if (_inputFramebuffers.empty()) return;

        if (!_enable) {
            Filter::update(frameTime);
            return;
        }

        Framebuffer *firstInputFramebuffer = _inputFramebuffers.begin()->second.frameBuffer;
        RotationMode firstInputRotation = _targetRotation;
        if (!firstInputFramebuffer) return;

        int rotatedFramebufferWidth = firstInputFramebuffer->getWidth();
        int rotatedFramebufferHeight = firstInputFramebuffer->getHeight();
        if (rotationSwapsSize(firstInputRotation)) {
            rotatedFramebufferWidth = firstInputFramebuffer->getHeight();
            rotatedFramebufferHeight = firstInputFramebuffer->getWidth();
        }

        bool needUpdateSize = false;
        if (lastInputHeight != rotatedFramebufferHeight) {
            lastInputWidth = rotatedFramebufferWidth;
            lastInputHeight = rotatedFramebufferHeight;
            needUpdateSize = true;
        }

        if (needUpdateSize) {
            updateTransformSize(rotatedFramebufferWidth, rotatedFramebufferHeight);
        }

        rotatedFramebufferWidth = _viewWidth;
        rotatedFramebufferHeight = _viewHeight;

        requestFrameBuffer(rotatedFramebufferWidth, rotatedFramebufferHeight);

        {
            proceed(frameTime);
        }

        returnFrameBuffer();


    }

    Framebuffer *OlaPreStreamingFilter::requestFrameBuffer(int width, int height) {
        if (_framebuffer == nullptr) {
            auto attributes = Framebuffer::defaultTextureAttribures;
            if (_useGray) {
                attributes.format = GL_LUMINANCE;
            }
            _framebuffer = getContext()->getFramebufferCache()->fetchFramebuffer(_context,
                                                                                 width,
                                                                                 height,
                                                                                 false,
                                                                                 attributes,
                                                                                 true);
        }

        return _framebuffer;
    }

    bool OlaPreStreamingFilter::downloadPixels(int width, int height) {
        // iOS 和 Android Hardwarebuffer 不再需要download
        if (_needDownload) {
            _framebuffer->lockAddress();
            auto *baseAddress = _framebuffer->frameBufferGetBaseAddress();
            int width_step = _framebuffer->getBytesPerRow();

            if (baseAddress != nullptr) {
                prepareFrameData(baseAddress, width, height, width_step);
            }
            _framebuffer->unlockAddress();
        } else {
            if (_useGray) {
                _format = (int)OlaImageFormat::GRAY8;
            } else {
                _format = (int)OlaImageFormat::SRGBA;
            }
        }
        return true;
    }

    void OlaPreStreamingFilter::returnFrameBuffer() {

    }


    bool OlaPreStreamingFilter::proceed(float frameTime, bool bUpdateTargets) {
#if DEBUG
		_framebuffer->lock(typeid(*this).name());
#else
		_framebuffer->lock();
#endif
        generateVBOBuffers();

        getContext()->setActiveShaderProgram(_filterProgram);


        if (_paddingColorRLocation == -1 || _paddingColorGLocation == -1 || _paddingColorBLocation == -1 ||
            _paddingFactorLocation == -1 || _colorMapUniformLocation == -1 || _texCoordAttributeLocation == -1) {
            _paddingColorRLocation = _filterProgram->getUniformLocation("paddingColorR");
            _paddingColorGLocation = _filterProgram->getUniformLocation("paddingColorG");
            _paddingColorBLocation = _filterProgram->getUniformLocation("paddingColorB");
            _paddingFactorLocation = _filterProgram->getUniformLocation("paddingFactor");
            _fixedOffsetLocation = _filterProgram->getUniformLocation("fixedOffset");
            _colorMapUniformLocation = _filterProgram->getUniformLocation("colorMap");
            _texCoordAttributeLocation = _filterProgram->getAttribLocation("texCoord");

        }

        _framebuffer->active();
        _filterProgram->setUniformValue(_fixedOffsetLocation, _fixedOffset);
        _filterProgram->setUniformValue(_paddingFactorLocation, _paddingFactor);
        _filterProgram->setUniformValue(_paddingColorRLocation, _paddingColorR);
        _filterProgram->setUniformValue(_paddingColorGLocation, _paddingColorG);
        _filterProgram->setUniformValue(_paddingColorBLocation, _paddingColorB);

        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, _context->vertexArray));
        CHECK_GL(glClearColor(_backgroundColor.r, _backgroundColor.g, _backgroundColor.b, _backgroundColor.a));
        CHECK_GL(glClear(GL_COLOR_BUFFER_BIT));
        int elementIndex = 0;

        auto texIdx = _inputFramebuffers.begin()->first;
        auto *fb = _inputFramebuffers.begin()->second.frameBuffer;


        if (fb == NULL) {
            return false;
        }
        CHECK_GL(glActiveTexture(GL_TEXTURE0 + texIdx));
        CHECK_GL(glBindTexture(GL_TEXTURE_2D, fb->getTexture()));

        _filterProgram->setUniformValue(_colorMapUniformLocation, texIdx);
        // texcoord attribute
        GLuint filterTexCoordAttribute = _texCoordAttributeLocation;
        if (filterTexCoordAttribute != (GLuint) -1) {
            CHECK_GL(glVertexAttribPointer(filterTexCoordAttribute, 2, GL_FLOAT, 0, 4 * sizeof(GLfloat), (void *) 8));
            CHECK_GL(glEnableVertexAttribArray(filterTexCoordAttribute));
        }

        elementIndex = _targetRotation;
        CHECK_GL(glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, 0, 4 * sizeof(GLfloat), (void *) 0));
        glEnableVertexAttribArray(_filterPositionAttribute);

        CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _context->elementArray[elementIndex]));
        CHECK_GL(glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (void *) 0));
        int width = _framebuffer->getWidth();
        int height = _framebuffer->getHeight();

        downloadPixels(width, height);

        CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
        CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        _framebuffer->inactive();
#if DEBUG
        _framebuffer->unlock(typeid(*this).name());
#else
		_framebuffer->unlock();
#endif
        unPrepear();
        return Source::proceed(frameTime, bUpdateTargets);
    }

    void OlaPreStreamingFilter::updateTransformSize(int rotatedFramebufferWidth, int rotatedFramebufferHeight) {

        if (_targetWidth == 0 && _targetHeight == 0) {
            //自适应缩放
            if (_maxSize != 0 || _minSize != 0) {
                if (_minSize > 0) {
                    //暂不实现
                } else if (_maxSize > 0) {
                    _paddingFactor = Vector2(0.0, 0.0); //不需要padding 直接拉伸
                    bool widthMax = rotatedFramebufferWidth > rotatedFramebufferHeight;
                    int maxLength = widthMax ? rotatedFramebufferWidth : rotatedFramebufferHeight;
                    int minLength = widthMax ? rotatedFramebufferHeight : rotatedFramebufferWidth;
                    float aspect = (float) maxLength / (float) minLength;
                    int scaleMinLength = (int) ((float) _maxSize / aspect);
                    if (_needFixedPadding) {
                       if (_scaleOfValue != 0) {
                            //缩放到scaleValue的倍数
                            int modValue = scaleMinLength % _scaleOfValue;
                            int divideValue = scaleMinLength / _scaleOfValue;

                            int fixMinSize =
                                    modValue > 0 ? (divideValue + 1) * _scaleOfValue : divideValue * _scaleOfValue;
                            if (widthMax) {
                                //宽是长边
                                _viewWidth = _maxSize;
                                _viewHeight = fixMinSize;
                            } else {
                                //高是长边
                                _viewWidth = fixMinSize;
                                _viewHeight = _maxSize;
                            }

                            if (_needPadding) {
                                //需要补齐灰度 这里计算一下PaddingFactor

                                int offset = fixMinSize - scaleMinLength;
                                float factor = (float) offset / (float) fixMinSize; //需要缩放的比例
                                if (widthMax) {
                                    _paddingFactor.y = factor;
                                } else {
                                    _paddingFactor.x = factor;
                                }
                            }
                        }

                    } else {
                        //不需要Padding 直接等比缩小
                        if (widthMax) {
                            _viewWidth = _maxSize;
                            _viewHeight = scaleMinLength;
                        } else {
                            _viewWidth = scaleMinLength;
                            _viewHeight = _maxSize;
                        }
                    }
                }

            } else {
                //必须给最长边不然bug 这个情况下 不缩放
                _viewWidth = rotatedFramebufferWidth;
                _viewHeight = rotatedFramebufferHeight;
            }

        } else {
            //指定大小缩放
            _viewWidth = _targetWidth;
            _viewHeight = _targetHeight;
            
            if (_flowFixedPadding) {
                
                float targetAspect = (float)_targetWidth / (float)_targetHeight;
                float myAspect = (float)rotatedFramebufferWidth / (float)rotatedFramebufferHeight;
                
                int newWidth = 0;
                int newHeight = 0;
                
                if (targetAspect == myAspect) {
                    //完全相等 不用补直接拉伸
                    _fixedWidthOffset = 0.0;
                    _fixedHeightOffset = 0.0;
                } else if (targetAspect > myAspect) {
                    // 目标更宽 向右补边
                    // 对齐高度 减宽度
                    
                    newHeight = _targetHeight;
                    newWidth = (int)((float)newHeight * myAspect);
                    _fixedWidthOffset = _viewWidth - newWidth;
                    _fixedOffset = Vector2((float)_fixedWidthOffset / (float)_targetWidth ,0.0);
                    
                } else {
                    // 目标更窄 向下补边
                    // 对齐宽度 减少高
                    newWidth = _targetWidth;
                    newHeight = (int)(float(newWidth) / myAspect);
                    _fixedHeightOffset = _viewHeight - newHeight;
                    _fixedOffset = Vector2(0.0, (float)_fixedHeightOffset / (float)_targetHeight);
                }
                
                
            }
        }
    }

}
