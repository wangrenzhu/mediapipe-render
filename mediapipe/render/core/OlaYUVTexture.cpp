#include "OlaYUVTexture.hpp"

NS_GI_BEGIN

#if defined(__APPLE__)
const std::string kYUVTextureFragmentShaderString = SHADER_STRING
(
 varying lowp vec2 vTexCoord;

 varying lowp vec2 vTexCoord1;
 
 uniform sampler2D colorMap;
 uniform sampler2D colorMap1;
 void main()
{
    mediump vec3 yuv;
    lowp vec3 rgb;
    
    yuv.x = texture2D(colorMap, vTexCoord).r;
    yuv.yz = texture2D(colorMap1, vTexCoord1).ra - vec2(0.5, 0.5);
    
    rgb = mat3(     1.0,    1.0,    1.0,
               0.0,    -0.343, 1.765,
               1.4,    -0.711, 0.0) * yuv;
    
    gl_FragColor = vec4(rgb, 1);
}
);

#else
//ITU-R BT.601 conversion
//R = 1.164*(Y-16) + 2.018*(Cr-128);
//G = 1.164*(Y-16) - 0.813*(Cb-128) - 0.391*(Cr-128);
//B = 1.164*(Y-16) + 1.596*(Cb-128);
const std::string kYUVTextureFragmentShaderString = SHADER_STRING
(
        precision mediump float;
        varying lowp vec2 vTexCoord;
        varying lowp vec2 vTexCoord1;


        uniform sampler2D colorMap;
        uniform sampler2D colorMap1;

        void main()
        {
            vec4 y = vec4((texture2D(colorMap, vTexCoord).r - 16./255.) * 1.164);
            vec4 u = vec4(texture2D(colorMap1, vTexCoord1).a - 128./255.);
            vec4 v = vec4(texture2D(colorMap1, vTexCoord1).r - 128./255.);
            y += v * vec4(1.596, -0.813, 0, 0);
            y += u * vec4(0, -0.392, 2.017, 0);
            y.a = 1.0;

            gl_FragColor = vec4(y.rgb, 1);
        }
);

#endif

OlaYUVTexture::OlaYUVTexture(Context *context) : Filter(context) {

}

OlaYUVTexture::~OlaYUVTexture() {

}

OlaYUVTexture* OlaYUVTexture::create(Context *context)
{
    OlaYUVTexture* ret = new (std::nothrow)OlaYUVTexture(context);
    if (!ret || !ret->init(context)) {
        delete ret;
        ret = 0;
    }
    return ret;
}

bool OlaYUVTexture::init(Context *context) {
    if (!Opipe::Filter::initWithFragmentShaderString(context, kYUVTextureFragmentShaderString, 2)) {
        return false;
    }
    return true;
}

NS_GI_END
