#ifndef OPIPE_FaceMeshModule
#define OPIPE_FaceMeshModule
#include <stdio.h>
#include <condition_variable>
#include "mediapipe/render/core/OlaContext.hpp"
#include "mediapipe/render/core/GLThreadDispatch.h"
#include "face_mesh_common.h"
#if defined(__APPLE__)
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import <CoreVideo/CoreVideo.h>
#import <UIKit/UIImage.h>
#elif defined(__ANDROID__) || defined(ANDROID)
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#endif

namespace Opipe
{
        class Source;
        class Filter;
        struct OMat
        {
                int width = 0;
                int height = 0;
                void *data = 0;
                int widthStep = 0;
                int channels = 4; //暂时只支持4
                bool create = false;
                OMat()
                {
                        channels = 0;
                }

                OMat(int w, int h, int ws)
                {
                        width = w;
                        height = h;
                        channels = 4;
                        widthStep = ws;
                        data = new char[widthStep * height];
                        memset(data, 0, sizeof(data));
                        create = true;
                }

                OMat(int w, int h)
                {
                        width = w;
                        height = h;
                        channels = 4;
                        if (w % 32 != 0)
                        {
                                widthStep = ((w / 32) + 1) * 32 * channels;
                        }
                        else
                        {
                                widthStep = w * channels;
                        }

                        data = new char[widthStep * height];
                        memset(data, 0, sizeof(data));
                        create = true;
                }

                OMat(int w, int h, char *d)
                {
                        width = w;
                        height = h;
                        channels = 4;
                        data = d;
                        if (w % 32 != 0)
                        {
                                widthStep = ((w / 32) + 1) * 32 * channels;
                        }
                        else
                        {
                                widthStep = w * channels;
                        }
                }

                void release()
                {
                        if (create)
                        {
                                delete (char *)data;
                        }
                        data = 0;
                }

                bool empty()
                {
                        return data == 0;
                }
        };

        class FaceMeshModule
        {
        public:
                FaceMeshModule();
                virtual ~FaceMeshModule();

                static FaceMeshModule *create();

                virtual OlaContext *currentContext() = 0;

                // 暂停渲染
                virtual void suspend() = 0;

                // 恢复渲染
                virtual void resume() = 0;

                virtual bool init(GLThreadDispatch *glDispatch, long glcontext, void *binaryData, int size) = 0;

                virtual void startModule() = 0;

                virtual void stopModule() = 0;
            
                /// 磨皮
                virtual float getSmoothing() = 0;
                
                /// 美白
                virtual float getWhitening() = 0;
            
                /// 瘦脸
                virtual float getSlim() = 0;
            
                virtual float getEye() = 0;
            
                /// 瘦鼻
                virtual float getNose() = 0;
            
                virtual void setSlim(float slim) = 0;
            
            
                virtual void setNose(float nose) = 0;
            
                virtual void setEye(float eye) = 0;
                
                /// 磨皮
                /// @param smoothing 磨皮 0.0 - 1.0
                virtual void setSmoothing(float smoothing) = 0;
                
                
                /// 美白
                /// @param whitening 美白 0.0 - 1.0
                virtual void setWhitening(float whitening) = 0;
            
                virtual bool getSegmentation() = 0;
            
                virtual void setSegmentationEnable(bool segEnable) = 0;

                virtual TextureInfo renderTexture(TextureInfo inputTexture) = 0;
            

#if defined(__APPLE__)
                virtual void processVideoFrame(CVPixelBufferRef pixelbuffer, int64_t timeStamp) = 0;
            
                virtual void setSegmentationBackground(UIImage *image) = 0;
#else
            
                virtual void setSegmentationBackground(OMat background) = 0;
#endif

                virtual void processVideoFrame(unsigned char *pixelBuffer,
                                               int size,
                                               int width,
                                               int height,
                                               int64_t timeStamp) = 0;

                virtual void processVideoFrame(int textureId,
                                       int width,
                                       int height,
                                       int64_t timeStamp) = 0;

                virtual void setInputSource(Source *source) = 0;

                virtual Filter* getOutputFilter() = 0;

                virtual void initLut(OMat &mat) = 0;
        };
}
#endif
