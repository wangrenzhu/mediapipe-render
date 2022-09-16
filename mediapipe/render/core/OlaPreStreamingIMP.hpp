//
//  OlaPreStreamingIMP.hpp
//  Quaramera
//
//  Created by wangrenzhu2021 on 2021/11/26.
//  Copyright © 2021 alibaba. All rights reserved.
//

#ifndef OlaPreStreamingIMP_hpp
#define OlaPreStreamingIMP_hpp

#include <stdio.h>
#include <vector>
#include "Source.hpp"
#include "OlaPreStreamingFilter.hpp"
#include "OlaPreStreaming.hpp"


namespace Opipe {
    class OlaPreStreamingIMP : public OlaPreStreaming {
    public:
        ~OlaPreStreamingIMP();
        
        OlaPreStreamingIMP(OpipeDispatch *dispatch);
        
        
        /// RGBA流预处理
        /// @param textureInfo RGBA纹理
        /// @param frameTime 帧时间
        virtual void preStream(OPreTextureInfo textureInfo, int64_t frameTime) override;

        virtual void reset() override;
        
        virtual int suspend() override;
        
        virtual int resume() override;
        
        /// 设置回调
        /// @param func 回调函数
        /// @param other userInfo
        /// @return 0 =失败 1=成功
        virtual int setCallBackHandler(const PreStreamDataCallback &func, void *other) override;
        
        /// 同步加载分流信息
        /// @param streamInfos streamInfos description
        virtual void loadStreamInfoSync(std::vector<OPreStreamInfo> streamInfos) override;
    
        virtual void attachTextureSource(Source *textureSource) override;
        
        virtual void detachTextureSource(Source *textureSource) override;
    private:
        std::unique_ptr<PreStreamCallbackHolder> _callbackHolder = 0;
        void *_holder;
        std::vector<OPreStreamInfo> _streamInfos;
    private:
        //GL 环境
        //GL 分流器
        std::vector<OlaPreStreamingFilter *> _paddingTransformFilters;
       
        OpipeDispatch *_dispatch = 0;
        Context *_context = 0;
        Framebuffer *_framebuffer = 0;
        std::mutex _resetMutex;
        Source *_textureSource = 0;
        bool _isPaused = false;
        bool _usePBO = false;
    };
}

#endif /* OlaPreStreamingIMP_hpp */
