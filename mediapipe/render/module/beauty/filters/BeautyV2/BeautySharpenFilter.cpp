#include "BeautySharpenFilter.hpp"

namespace Opipe
{
    const std::string kBeautySharpenFilterVertexShaderString = SHADER_STRING(
        attribute vec4 position;
        attribute vec4 texCoord;
        
        uniform float imageWidthFactor; 
        uniform float imageHeightFactor; 
        uniform float sharpness;
        
        varying vec2 textureCoordinate;
        varying vec2 leftTextureCoordinate;
        varying vec2 rightTextureCoordinate; 
        varying vec2 topTextureCoordinate;
        varying vec2 bottomTextureCoordinate;
        
        varying float centerMultiplier;
        varying float edgeMultiplier;
        
        void main()
        {
            gl_Position = position;
            
            vec2 widthStep = vec2(imageWidthFactor, 0.0);
            vec2 heightStep = vec2(0.0, imageHeightFactor);
            
            textureCoordinate = texCoord.xy;
            leftTextureCoordinate = texCoord.xy - widthStep;
            rightTextureCoordinate = texCoord.xy + widthStep;
            topTextureCoordinate = texCoord.xy + heightStep;     
            bottomTextureCoordinate = texCoord.xy - heightStep;
            
            centerMultiplier = 1.0 + 4.0 * sharpness;
            edgeMultiplier = sharpness;
    });


    const std::string kBeautySharpenFilterFragmentShaderString = SHADER_STRING(
        precision highp float;
 
        varying highp vec2 textureCoordinate;
        varying highp vec2 leftTextureCoordinate;
        varying highp vec2 rightTextureCoordinate; 
        varying highp vec2 topTextureCoordinate;
        varying highp vec2 bottomTextureCoordinate;
        
        varying highp float centerMultiplier;
        varying highp float edgeMultiplier;

        uniform sampler2D colorMap;
        
        void main()
        {
            mediump vec3 textureColor = texture2D(colorMap, textureCoordinate).rgb;
            mediump vec3 leftTextureColor = texture2D(colorMap, leftTextureCoordinate).rgb;
            mediump vec3 rightTextureColor = texture2D(colorMap, rightTextureCoordinate).rgb;
            mediump vec3 topTextureColor = texture2D(colorMap, topTextureCoordinate).rgb;
            mediump vec3 bottomTextureColor = texture2D(colorMap, bottomTextureCoordinate).rgb;

            gl_FragColor = vec4((textureColor * centerMultiplier - 
                                (leftTextureColor * edgeMultiplier + rightTextureColor * edgeMultiplier +
                                topTextureColor * edgeMultiplier + bottomTextureColor * edgeMultiplier)), 
                                texture2D(colorMap, bottomTextureCoordinate).w);
    });

    BeautySharpenFilter::BeautySharpenFilter(Context *context) : Filter(context)
    {

    }

    BeautySharpenFilter* BeautySharpenFilter::create(Context *context)
    {
        BeautySharpenFilter *ret = new (std::nothrow)BeautySharpenFilter(context);
        if (ret && !ret->init(context)) {
            delete ret;
            ret = 0;
        }
        return ret;
    }

    bool BeautySharpenFilter::proceed(float frameTime,
                                        bool bUpdateTargets /* = true*/)
    {
        _filterProgram->setUniformValue("sharpness", _sharpness);
        _filterProgram->setUniformValue("imageWidthFactor", _widthFactor);
        _filterProgram->setUniformValue("imageHeightFactor", _heightFactor);
        return Filter::proceed(frameTime, bUpdateTargets);
    }

    bool BeautySharpenFilter::init(Context *context)
    {
        if (!Filter::initWithShaderString(context, kBeautySharpenFilterVertexShaderString, 
                                          kBeautySharpenFilterFragmentShaderString))
        {
            return false;
        }
        return true;
    }

    void BeautySharpenFilter::update(float frameTime) {
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
        
        _widthFactor = 1.0 / rotatedFramebufferWidth;
        _heightFactor = 1.0 / rotatedFramebufferHeight;
        Filter::update(frameTime);
    }
}
