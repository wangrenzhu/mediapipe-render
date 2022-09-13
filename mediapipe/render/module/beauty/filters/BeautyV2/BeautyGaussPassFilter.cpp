#include "BeautyGaussPassFilter.hpp"

namespace Opipe
{
    const std::string kBeautyGaussPassFilterVertexShaderString = SHADER_STRING
    (
     attribute vec4 position;
     attribute vec4 texCoord;
    
    const int SHIFT_SIZE = 5;
    
    uniform highp float texelWidthOffset;
    uniform highp float texelHeightOffset;

    varying vec2 textureCoordinate;
    varying vec4 blurShiftCoordinates[SHIFT_SIZE];
    
    void main()
    {
        gl_Position = position;
        textureCoordinate = texCoord.xy;
        // 偏移步距
        mediump vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);
        // 记录偏移坐标
        for (int i = 0; i < SHIFT_SIZE; i++) {
            blurShiftCoordinates[i] = vec4(textureCoordinate.xy - float(i + 1) * singleStepOffset,
                                           textureCoordinate.xy + float(i + 1) * singleStepOffset);
        }
    });

    const std::string kBeautyGaussPassFilterFragmentShaderString = SHADER_STRING
    (
    precision mediump float;
    
    varying vec2 textureCoordinate;
    uniform sampler2D colorMap;
    const int SHIFT_SIZE = 5;
    varying vec4 blurShiftCoordinates[SHIFT_SIZE];
    
    void main()
    {
        // 计算当前坐标的颜色值
        vec4 currentColor = texture2D(colorMap, textureCoordinate);
        mediump vec3 sum = currentColor.rgb;
        // 计算偏移坐标的颜色值总和
        for (int i = 0; i < SHIFT_SIZE; i++) {
            sum += texture2D(colorMap, blurShiftCoordinates[i].xy).rgb;
            sum += texture2D(colorMap, blurShiftCoordinates[i].zw).rgb;
        }
        // 求出平均值
        gl_FragColor = vec4(sum * 1.0 / float(2 * SHIFT_SIZE + 1), currentColor.a);
    });

    BeautyGaussPassFilter::BeautyGaussPassFilter(Context *context, Type type) : Filter(context), _type(type) {

    }

    BeautyGaussPassFilter* BeautyGaussPassFilter::create(Context *context, Type type) {
        BeautyGaussPassFilter *ret = new (std::nothrow) BeautyGaussPassFilter(context, type);
        if (!ret || !ret->init(context))
        {
            delete ret;
            ret = 0;
        }

        return ret;
    }

    bool BeautyGaussPassFilter::init(Context *context)
    {
        if (!Filter::initWithShaderString(_context, kBeautyGaussPassFilterVertexShaderString, kBeautyGaussPassFilterFragmentShaderString))
        {
            return false;
        }

        _texelWidthOffsetLocation = _filterProgram->getUniformLocation("texelWidthOffset");
        _texelHeightOffsetLocation = _filterProgram->getUniformLocation("texelHeightOffset");
        return true;
    }

    void BeautyGaussPassFilter::update(float frameTime) {
        Framebuffer* firstInputFramebuffer = _inputFramebuffers.begin()->second.frameBuffer;
        RotationMode firstInputRotation = _inputFramebuffers.begin()->second.rotationMode;
        if (!firstInputFramebuffer) return;

        int rotatedFramebufferWidth = firstInputFramebuffer->getWidth();
        int rotatedFramebufferHeight = firstInputFramebuffer->getHeight();
        if (rotationSwapsSize(firstInputRotation))
        {
            rotatedFramebufferWidth = firstInputFramebuffer->getHeight();
            rotatedFramebufferHeight = firstInputFramebuffer->getWidth();
        }

        if (_framebufferScale !=  1.0) {
            rotatedFramebufferWidth = int(rotatedFramebufferWidth * _framebufferScale);
            rotatedFramebufferHeight = int(rotatedFramebufferHeight * _framebufferScale);
        }
        
        if (_type == HORIZONTAL) {
            setTexelOffsetSize(rotatedFramebufferWidth, 0);
        } else {
            setTexelOffsetSize(0, rotatedFramebufferHeight);
        }
        Filter::update(frameTime);
    }

    bool BeautyGaussPassFilter::proceed(float frameTime,
                                        bool bUpdateTargets /* = true*/)
    {
        _filterProgram->setUniformValue(_texelWidthOffsetLocation, _texelWidthOffset);
        _filterProgram->setUniformValue(_texelHeightOffsetLocation, _texelHeightOffset);
        return Filter::proceed(frameTime, bUpdateTargets);
    }
}
