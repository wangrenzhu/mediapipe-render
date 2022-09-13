#include "BeautyHighPassFilter.hpp"

namespace Opipe
{
    const std::string kBeautyHighPassFilterFragmentShaderString = SHADER_STRING(
        // 美肤滤镜
        precision mediump float;
        varying highp vec2 vTexCoord;
        uniform sampler2D colorMap; // 输入原图
        uniform sampler2D colorMap1;  // 高斯模糊图片

        const float intensity = 24.0;   // 强光程度

        void main() {
            lowp vec4 sourceColor = texture2D(colorMap, vTexCoord);
            lowp vec4 blurColor = texture2D(colorMap1, vTexCoord);
            // 高通滤波之后的颜色值
            highp vec4 highPassColor = sourceColor - blurColor;
            // 对应混合模式中的强光模式(color = 2.0 * color1 * color2)，对于高反差的颜色来说，color1 和color2 是同一个
            highPassColor.r = clamp(2.0 * highPassColor.r * highPassColor.r * intensity, 0.0, 1.0);
            highPassColor.g = clamp(2.0 * highPassColor.g * highPassColor.g * intensity, 0.0, 1.0);
            highPassColor.b = clamp(2.0 * highPassColor.b * highPassColor.b * intensity, 0.0, 1.0);
            // 输出的是把痘印等过滤掉
            gl_FragColor = vec4(highPassColor.rgb, 1.0);
    });

    BeautyHighPassFilter::BeautyHighPassFilter(Context *context) : Filter(context)
    {

    }

    BeautyHighPassFilter* BeautyHighPassFilter::create(Context *context)
    {
        BeautyHighPassFilter *ret = new (std::nothrow)BeautyHighPassFilter(context);
        if (ret && !ret->init(context)) {
            delete ret;
            ret = 0;
        }
        return ret;
    }

    bool BeautyHighPassFilter::init(Context *context)
    {
        if (!Filter::initWithFragmentShaderString(context, kBeautyHighPassFilterFragmentShaderString, 2))
        {
            return false;
        }
        return true;
    }
}
