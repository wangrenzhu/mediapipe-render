//
//  OlaPreStreamingFilter.hpp
//  Quaramera
//
//  Created by wangrenzhu on 2021/4/16.
//  Copyright © 2021 alibaba. All rights reserved.
//

#ifndef OlaPreStreamingFilter_hpp
#define OlaPreStreamingFilter_hpp

#include <stdio.h>
#include "Filter.hpp"
#include "Context.hpp"
#include "OlaImageFrame.hpp"


namespace Opipe {


    extern const char kOlaPreStreamingFilterFragmentShaderString[];
    extern const char kOlaPreStreamingFilterVertexShaderString[];


    /// 将任意图缩放到指定最大宽度的大小，最小边等比例缩放后按照32倍数补齐
    class OlaPreStreamingFilter : public Filter {
    public:
        static OlaPreStreamingFilter *create(Context *context, bool useGray = false);

        virtual bool proceed(float frameTime = 0, bool bUpdateTargets = true) override;

        virtual void update(float frameTime) override;

        Vector2 getRenderDataSize() {
            return Vector2(_viewWidth, _viewHeight);
        };

        std::string &getStreamName() {
            return _streamName;
        };

        bool useGrayBuffer() {
            return _useGray;
        }

        int getWidthStep() {
            return _widthStep;
        }
        
        bool getUseGPU() {
            return _useGPU;
        }

        void setScaleOfValue(int scaleOf) {
            _scaleOfValue = scaleOf;
        }

        void setMaxSize(int maxSize) {
            _maxSize = maxSize;
        }

        void setMinSize(int minSize) {
            _minSize = minSize;
        }

        void setNeedPadding(bool needPadding) {
            _needPadding = needPadding;
        }

        void setNeedFixedPadding(bool fixedPadding) {
            _needFixedPadding = fixedPadding;
        }
        
        void setFlowFixedPadding(bool fixedPadding) {
            _flowFixedPadding = fixedPadding;
        }

        void setGray(bool useGray) {
            _useGray = useGray;
            _useGrayLocation = _useGray ? 1 : 0;
        }
        
        void setUseGPU(bool useGPU) {
            _useGPU = useGPU;
        }

        void setTargetWidth(int targetWidth) {
            _targetWidth = targetWidth;
        }

        void setTargetHeight(int targetHeight) {
            _targetHeight = targetHeight;
        }


        void setStreamName(std::string name) {
            _streamName = name;
        }

        void setPaddingColor(float r, float g, float b) {
            _paddingColorR = r;
            _paddingColorG = g;
            _paddingColorB = b;
        }

        void setNeedDownloadPixels(bool value) {
            _needDownload = value;
        }

        void lock() {
            isLocked = true;
        }

        void unlock() {
            isLocked = false;
        }
        
        int format() {
            return _format;
        }
        
        int fixedHeightOffset() {
            return _fixedHeightOffset;
        }
        
        int fixedWidthOffset() {
            return _fixedWidthOffset;
        }

        OlaPreStreamingFilter(Context *context);

        ~OlaPreStreamingFilter();

        Framebuffer *getFramebuffer() {
            return _framebuffer;
        }

        std::unique_ptr<OlaImageFrame> cacheData;

    protected:

        virtual Framebuffer *requestFrameBuffer(int width, int height);

        virtual void returnFrameBuffer();

        virtual bool downloadPixels(int width, int height);

        void prepareFrameData(void *data, int width, int height, int width_step);

        virtual bool init(Context *context);

        int _viewWidth = 0; //根据targetMaxWidth计算出的结果
        int _viewHeight = 0;
        int _widthStep = 0;


    private:

        int _targetWidth = 0;   //指定大小
        int _targetHeight = 0;  //指定大小

        int _paddingValue = 0; //Padding的值
        int _scaleOfValue = 0; // 缩放倍数
        int _maxSize = 0;
        int _minSize = 0;
        bool _needPadding = false;  //按比例缩放并按scaleOf 补齐，但不留灰边
        bool _needFixedPadding = false; //同上但留灰边
        bool _useGray = false; //启用灰度图
        bool _useGPU = false; //是否使用GPU纹理作为数据输出
        bool _needDownload = false;
        bool _flowFixedPadding = false; // 是否需要等比缩放补像素
        
        int _fixedHeightOffset = 0; // FlowFixedPadding 为true的时候生效
        int _fixedWidthOffset = 0;
        std::string _streamName = "";
        int lastInputWidth = 0;
        int lastInputHeight = 0;

        void updateTransformSize(int rotatedFramebufferWidth, int rotatedFramebufferHeight);


    private:
        GLuint _useGrayLocation = 0;
        Vector2 _paddingFactor = Vector2(0.0, 0.0);
        Vector2 _fixedOffset = Vector2(0.0, 0.0); //右或者下黑边
        float _paddingColorR = 0.0;
        float _paddingColorG = 0.0;
        float _paddingColorB = 0.0;
        GLuint _fixedOffsetLocation = -1;
        GLuint _paddingFactorLocation = -1;
        GLuint _paddingColorRLocation = -1;
        GLuint _paddingColorGLocation = -1;
        GLuint _paddingColorBLocation = -1;
        GLuint _colorMapUniformLocation = -1;
        GLuint _texCoordAttributeLocation = -1;
        int _format = 0;
        bool isLocked = false;

    };
}

#endif /* QAPaddingTrasnformFilter_hpp */
