#include "OlaSegmentOutlineFilter.hpp"

namespace Opipe
{
    const std::string kOlaSegmentOutlineFragmentShaderString = SHADER_STRING(
        varying lowp vec2 vTexCoord;
        uniform sampler2D colorMap;
        uniform sampler2D colorMap1;
        uniform lowp vec2 iResolution;
        const highp float Pi = 3.1415926535;
        precision lowp float;
        lowp mat2 rot(float a) {
            return mat2(cos(a), sin(a), -sin(a), cos(a));
        }

        lowp vec4 drawOutline(vec2 uv, sampler2D maskTexture) {
            lowp vec4 tex = texture2D(maskTexture, uv).aaaa;
            lowp vec3 color = vec3(0.0);
            lowp float dist = 2. / iResolution.x;
            
            int N = 2;
            lowp vec4 outline = vec4(0);
            for(int i = 0; i < N; ++i) {
                lowp vec2 dir = rot(float(i) * 2. * Pi / float(N)) * vec2(dist, 0);
                vec4 t = texture2D(maskTexture, uv + dir);
                outline.rgb += color * t.a;
                outline.a += t.a;
            }
            tex = outline;
            return tex;
        }


        void main() {
            lowp vec4 foreColor = texture2D(colorMap, vTexCoord);
            lowp vec4 maskColor = drawOutline(vTexCoord, colorMap1);
//            lowp vec4 maskColor = texture2D(colorMap1, vTexCoord);
            lowp vec4 backgroundColor = vec4(0.0, 1.0, 0.0, 1.0); //绿色
            lowp float factor = clamp(maskColor.a, 0.0, 1.0);
//            lowp float factor = maskColor.a;
            lowp vec4 color = mix(foreColor, backgroundColor, 1.0 - factor);
            gl_FragColor = color;    
    });

    OlaSegmentOutlineFilter::OlaSegmentOutlineFilter(Context *context) : Filter(context)
    {
    }

    OlaSegmentOutlineFilter *OlaSegmentOutlineFilter::create(Context *context)
    {
        OlaSegmentOutlineFilter *ret =
            new (std::nothrow) OlaSegmentOutlineFilter(context);
        if (!ret || !ret->init(context))
        {
            delete ret;
            ret = 0;
        }

        return ret;
    }

    bool OlaSegmentOutlineFilter::init(Context *context)
    {
        if (!Filter::initWithFragmentShaderString(context, kOlaSegmentOutlineFragmentShaderString, 2))
        {
            return false;
        }
        return true;
    }

    bool OlaSegmentOutlineFilter::proceed(float frameTime,
                                        bool bUpdateTargets /* = true*/)
    {
        return Filter::proceed(frameTime, bUpdateTargets);
    }

    void OlaSegmentOutlineFilter::setInputFramebuffer(Framebuffer* framebuffer,
                                                      RotationMode rotationMode,
                                                      int texIdx,
                                                      bool ignoreForPrepared) {
        Target::setInputFramebuffer(framebuffer, rotationMode, texIdx, ignoreForPrepared);
    }
}
