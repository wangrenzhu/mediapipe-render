#include "BeautyAdjustFilter.hpp"

namespace Opipe
{

    const std::string kBeautyAdjustFilterFragmentShaderString =
    SHADER_STRING
    (
     precision mediump float;
     varying highp vec2 vTexCoord;
     uniform sampler2D colorMap;
     uniform sampler2D colorMap1;
     uniform sampler2D colorMap2;
     uniform lowp float intensity;
     lowp float factor1 = 2.782;
     lowp float factor2 = 1.131;
     lowp float factor3 = 1.158;
     lowp float factor4 = 2.901;
     lowp float factor5 = 0.979;
     lowp float factor6 = 0.639;
     lowp float factor7 = 0.963;
     
     lowp vec3 rgb2hsv(lowp vec3 c) {
        lowp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        lowp vec3 hsv = vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        return hsv;
    }
     
     lowp vec3 ContrastSaturationBrightness(lowp vec3 color, lowp float brt, lowp float sat, lowp float con) {
        const lowp float AvgLumR = 0.5;
        const lowp float AvgLumG = 0.5;
        const lowp float AvgLumB = 0.5;
        const lowp vec3 LumCoeff = vec3(0.2125, 0.7154, 0.0721);
        lowp vec3 AvgLumin = vec3(AvgLumR, AvgLumG, AvgLumB);
        lowp vec3 brtColor = color * brt;
        lowp vec3 intensityB = vec3(dot(brtColor, LumCoeff));
        lowp vec3 satColor = mix(intensityB, brtColor, sat);
        lowp vec3 conColor = mix(AvgLumin, satColor, con);
        return conColor;
    }
     
     void main() {
        lowp vec4 inputColor = texture2D(colorMap, vTexCoord);
        lowp vec4 bColor = texture2D(colorMap1, vTexCoord);
        lowp vec4 highPassBlurColor = texture2D(colorMap2, vTexCoord);
        // 调节蓝色通道值
        mediump float value = clamp((min(inputColor.b, bColor.b) - 0.2) * 5.0, 0.0, 1.0);
        // 找到模糊之后RGB通道的最大值
        mediump float maxChannelColor = max(max(highPassBlurColor.r, highPassBlurColor.g), highPassBlurColor.b);
        // 计算当前的强度
        mediump float currentIntensity = (1.0 - maxChannelColor / (maxChannelColor + 0.2)) * value * intensity;
        // 混合输出结果
        lowp vec3 resultColor = mix(inputColor.rgb, bColor.rgb, currentIntensity);
        // 输出颜色
        //            gl_FragColor = vec4(resultColor, 1.0);
        lowp vec3 hsv = rgb2hsv(inputColor.rgb);
        lowp float opacityLimit = 1.0;
        if ((0.18 <= hsv.x && hsv.x <= 0.89) || hsv.z <= 0.2)
        {
            opacityLimit = 0.0;
        }
        if (0.16 < hsv.x && hsv.x < 0.18)
        {
            opacityLimit = min(opacityLimit, (0.18 - hsv.x) / 0.02);
        }
        
        if (0.89 < hsv.x && hsv.x < 0.91)
        {
            opacityLimit = min(opacityLimit, 1.0 - (0.91 - hsv.x) / 0.02);
        }
        if (0.2 < hsv.z && hsv.x < 0.3)
        {
            opacityLimit = min(opacityLimit, 1.0 - (0.3 - hsv.z) / 0.1);
        }
        if (opacityLimit == 0.0)
        {
            gl_FragColor = inputColor;
            return;
        }
        vec4 blurColor = vec4(resultColor, 1.0);
        lowp float cDistance = distance(vec3(0.0, 0.0, 0.0), max(blurColor.rgb - inputColor.rgb, 0.0)) * factor1;
        lowp vec3 brightColor = ContrastSaturationBrightness(inputColor.rgb, factor2, 1.0, factor3);
        lowp vec3 mix11Color = mix(inputColor.rgb, brightColor.rgb, cDistance);
        lowp float dDistance = distance(vec3(0.0, 0.0, 0.0), max(inputColor.rgb - blurColor.rgb, 0.0)) * factor4;
        lowp vec3 darkColor = ContrastSaturationBrightness(inputColor.rgb, factor5, 1.0, factor6);
        lowp vec3 mix115Color = mix(mix11Color.rgb, darkColor.rgb, dDistance);
        lowp vec3 mix12Color;
        lowp vec3 mix116Color;
        if (factor7 < 0.999)
        {
            mix116Color = mix(inputColor.rgb, mix115Color.rgb, factor7);
            
            mix12Color = mix(mix116Color.rgb, blurColor.rgb, opacityLimit);
        }
        else
        {
            mix12Color = mix(mix115Color.rgb, blurColor.rgb, opacityLimit);
        }
        if (intensity < 0.999)
        {
            float newAlpha = intensity < 0.0 ? 0.0 : intensity;
            gl_FragColor = vec4(mix(inputColor.rgb, mix12Color.rgb, newAlpha), inputColor.a);
        }
        else
        {
            gl_FragColor = vec4(mix12Color.rgb, inputColor.a);
        }
    });

    BeautyAdjustFilter::BeautyAdjustFilter(Context *context) : Filter(context), _indensity(0.8)
    {
    }

    BeautyAdjustFilter *BeautyAdjustFilter::create(Context *context)
    {
        BeautyAdjustFilter *ret =
        new (std::nothrow) BeautyAdjustFilter(context);
        if (!ret || !ret->init(context))
        {
            delete ret;
            ret = 0;
        }
        
        return ret;
    }

    bool BeautyAdjustFilter::init(Context *context)
    {
        if (!Filter::initWithFragmentShaderString(context, kBeautyAdjustFilterFragmentShaderString, 3))
        {
            return false;
        }
        return true;
    }

    bool BeautyAdjustFilter::proceed(float frameTime,
                                     bool bUpdateTargets /* = true*/)
    {
        _filterProgram->setUniformValue("intensity", _indensity);
        return Filter::proceed(frameTime, bUpdateTargets);
    }
}
