#ifndef OPIPE_FaceMeshModule_Impl
#define OPIPE_FaceMeshModule_Impl

#include "mediapipe/render/module/common/ola_graph.h"
#include "mediapipe/render/core/OpipeDispatch.hpp"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/render/core/Context.hpp"
#include "mediapipe/render/core/Source.hpp"
#include "mediapipe/render/core/OlaCameraSource.hpp"
#include "face_mesh_module.h"
#include "face_mesh_beauty_render.h"


namespace Opipe
{
    class FaceMeshModuleIMP;
    class FaceMeshCallFrameDelegate : public MPPGraphDelegate
    {
    public:
        FaceMeshCallFrameDelegate();
        ~FaceMeshCallFrameDelegate();

        void outputPacket(OlaGraph *graph, const mediapipe::Packet &packet,
                          MPPPacketType packetType, const std::string &streamName) override;

        void attach(FaceMeshModuleIMP *imp) {
            _imp = imp;
        }
        OpipeProfile currentProfile() {
             return _profile;
        }
        
    private:
        int64_t _last_landmark_ts = 0;
        bool _hasFace = false;
        NormalizedLandmarkList _lastLandmark;
        NormalizedLandmarkList _emptyLandmark;
        FaceMeshModuleIMP *_imp = nullptr;
        OpipeProfile _profile;
    };

    class FaceMeshModuleIMP : public FaceMeshModule
    {
    public:
        FaceMeshModuleIMP();
        ~FaceMeshModuleIMP();

        // 暂停渲染
        virtual void suspend() override;

        // 恢复渲染
        virtual void resume() override;

        // env iOS给空
        virtual bool init(GLThreadDispatch *glDispatch, long glcontext, void *binaryData, int size, 
                          bool useBeautyV2) override;

        virtual void startModule() override;

        virtual void stopModule() override;

        virtual void initLut(OMat &mat, OMat &grayMat) override;

        // 外部共享Context用
        virtual OlaContext* currentContext() override {
            return _olaContext;
        } 

#if defined(__APPLE__)

        /// 算法流输入
        /// @param pixelbuffer pixelbuffer description
        /// @param timeStamp timeStamp description
        virtual void processVideoFrame(CVPixelBufferRef pixelbuffer, int64_t timeStamp) override;

        virtual void setSegmentationBackground(UIImage *image) override;
#else
        virtual void setSegmentationBackground(OMat background) override;
#endif

        virtual void processVideoFrame(unsigned char *pixelbuffer,
                                       int size,
                                       int width,
                                       int height,
                                       int64_t timeStamp) override;

        virtual void processVideoFrame(int textureId,
                                       int width,
                                       int height,
                                       int64_t timeStamp) override;


        virtual TextureInfo renderTexture(TextureInfo inputTexture) override;
        
        virtual void setLandmark(NormalizedLandmarkList landmark, int64_t timestamp);
        
        /// 磨皮
        float getSmoothing() override {
            return _render->getSmoothing();
        }
        
        /// 美白
        float getWhitening() override {
            return _render->getWhitening();
        }
        
        float getEye() override {
            return _render->getEye();
        }
        
        float getSlim() override {
            return _render->getFace();
        }
        
        float getNose() override {
            return _render->getNose();
        }
        
        float getSharpness() override {
            return _render->getSharpness();
        }
        
        /// 磨皮
        /// @param smoothing 磨皮 0.0 - 1.0
        void setSmoothing(float smoothing) override {
            _render->setSmoothing(smoothing);
        }
        
        /// 美白
        /// @param whitening 美白 0.0 - 1.0
        void setWhitening(float whitening) override {
            _render->setWhitening(whitening);
        }
        
        void setEye(float eye) override {
            _render->setEye(eye);
        }
        
        void setSlim(float slim) override {
            _render->setFaceSlim(slim);
        }
        
        void setNose(float nose) override {
            _render->setNoseFactor(nose);
        }
        
        void setSharpness(float sharpness) override {
            _render->setSharpness(sharpness);
        }
        
        bool getSegmentation() override {
            return _segEnable;
        }
        
        void setSegmentationEnable(bool segEnable) override;
        
        OpipeDispatch* currentDispatch() {
            return _dispatch.get();
        }

        virtual void setInputSource(Source *source) override;

        Filter* getOutputFilter() override;

        SourceCamera *getOutputSource() {
            return _outputSource;
        }

        virtual OpipeProfile currentProfile() override {
             return _delegate->currentProfile();
        }

    private:
        std::unique_ptr<OpipeDispatch> _dispatch;
        std::unique_ptr<OlaGraph> _graph;
        Context *_context = nullptr;
        bool _isInit = false;
        bool _segEnable = false;
        bool _useBeautyV2 = false;
        NormalizedLandmarkList _lastLandmark;
        std::shared_ptr<FaceMeshCallFrameDelegate> _delegate;
        FaceMeshBeautyRender *_render = nullptr;
        OlaContext *_olaContext = nullptr;
        Timestamp _lastTs = Timestamp::Unset();
        OMat _omat;
        OMat _grayMat;
        OlaCameraSource *_inputSource = nullptr; // 处理输入
        SourceCamera *_outputSource = nullptr; //处理输出
    };
}
#endif
