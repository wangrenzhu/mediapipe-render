//
//  OPreStreamingCalculator.hpp
//  Quaramera
//
//  Created by wangrenzhu2021 on 2021/11/25.
//  Copyright © 2021 alibaba. All rights reserved.
//

#ifndef OPreStreamingCalculator_hpp
#define OPreStreamingCalculator_hpp

#include <stdio.h>
#include <functional>
#include <string>
#include "Context.hpp"
#include "Source.hpp"
#include "OpipeDispatch.hpp"

#if defined(__APPLE__)
#include <AVFoundation/AVFoundation.h>
#include <OpenGLES/EAGL.h>
#include <OpenGLES/ES3/gl.h>
#else

#include "EGLAndroid.h"

#include <EGL/egl.h>
#include <GLES3/gl3.h>

#endif
namespace Opipe {
 
    typedef enum {
      OPTypeCPUImageFrame,  //实际的数据
      OPTypeGPUbuffer,  //纹理ID
    } OPDataType;

    struct OPreTextureInfo {
        /**
         * 纹理需要旋转的角度，勇于显示纹理
         */
        RotationMode rotationMode = NoRotation;
        /**
         * 设备旋转角度，算法可能对输入分流有视觉正方向要求，所以通过deviceRotation将角度修正
         */
        RotationMode deviceRotationMode = NoRotation;
        
        TextureAttributes textureAttributes;
        
        int width = 0 ;
        int height = 0 ;
        GLuint textureId = -1;  //相机纹理&视频纹理
        int64_t frameTime = 0;
#if defined(__APPLE__)
        IOSurfaceID surfaceId = -1;
#endif
        
    };

    typedef struct OPreStreamInfo {
        std::string name = "";
        OPDataType type = OPTypeCPUImageFrame;
        int width = 0;
        int height = 0;
        int scaleOf = 0;
        int maxSize = 0;
        int minSize = 0;
        int flowFixedPadding = 0;   //是否要宽高自适应步黒边
        int needFixedPadding = 0;
        int needPadding = 0;
        int useGray = 0;
        struct Color {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };
        Color padding_color = {};
    } OPreStreamInfo;

    typedef struct PreStreamFrameData {
        bool hasData = false;
        bool isGPUData = false;
    #if defined(__APPLE__)
        CVPixelBufferRef pixelBuffer = 0;
    #endif
        char *dataBuffers = nullptr;
        int width = 0;
        int height = 0;
        int fixedHeightOffset = 0;  // 如果图是横屏 则补高 底部
        int fixedWidthOffset = 0;   // 如果图是竖屏 则补宽 右边
        int width_step = 0;
        GLuint textureId = -1;
        int surfaceId = -1;
        int64_t timeStamp = -1;
        std::string stream_name; //分流的名称
        TextureAttributes textureAtributes;
        bool needDelete = false;
        int64_t analysisTimeStamp = -1;
    } PreStreamFrameData;

    typedef std::function<void(int64_t, std::vector<PreStreamFrameData>, void*)> PreStreamDataCallback;

    typedef struct {
        PreStreamDataCallback func;
        void *other;
    } PreStreamCallbackHolder;

    class OlaPreStreaming {
    public:
        virtual ~OlaPreStreaming() {}
        
        static TextureAttributes defaultRGBATextureAttribures;

        /// 通过QStreamingContext 创建一个分流器
        /// @param dispatch 执行器
        static OlaPreStreaming* create(OpipeDispatch *dispatch);
        
        /// 分流上屏RGBA的纹理
        virtual void preStream(OPreTextureInfo textureInfo, int64_t frameTime) = 0;

        virtual void reset() = 0;
        
        virtual int suspend() = 0;
        
        virtual int resume() = 0;

        /// 设置分流数据回调
        /// @param func 回调函数
        /// @param other 透传
        virtual int setCallBackHandler(const PreStreamDataCallback &func, void *other) = 0;
        
        /// 同步加载分流信息
        /// @param streamInfos streamInfos description
        virtual void loadStreamInfoSync(std::vector<OPreStreamInfo> streamInfos) = 0;
        
        virtual void attachTextureSource(Source *textureSource) = 0;
        
        virtual void detachTextureSource(Source *textureSource) = 0;
    };
}

#endif /* OPreStreamingCalculator_hpp */
 
