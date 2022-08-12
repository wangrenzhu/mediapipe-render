#ifndef OPIPE_FaceMeshModule_Impl
#define OPIPE_FaceMeshModule_Impl

#include "mediapipe/render/module/common/ola_graph.h"
#include "mediapipe/render/core/OpipeDispatch.hpp"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/render/core/Context.hpp"
#include "mediapipe/render/core/Source.hpp"
#include "face_mesh_module.h"
#include "face_mesh_beauty_render.h"

#define TestTemplateFace 0
#if TestTemplateFace
#include "mediapipe/render/core/SourceImage.hpp"
#endif


namespace Opipe
{
    class FaceMeshModuleIMP;
    class FaceMeshCallFrameDelegate : public MPPGraphDelegate
    {
    public:
        FaceMeshCallFrameDelegate();
        ~FaceMeshCallFrameDelegate();
#if defined(__APPLE__)
        void outputPixelbuffer(OlaGraph *graph, CVPixelBufferRef pixelbuffer,
                               const std::string &streamName, int64_t timstamp) override;
#endif
        void outputPacket(OlaGraph *graph, const mediapipe::Packet &packet,
                          MPPPacketType packetType, const std::string &streamName) override;
        
        void outputPacket(OlaGraph *graph, const mediapipe::Packet &packet,
                          const std::string &streamName) override;

        void outputPacket(OlaGraph *graph,
                                  const mediapipe::Gpubuffer &gpubuffer,
                                  const std::string &streamName) override;
        
        void attach(FaceMeshModuleIMP *imp) {
            _imp = imp;
        }
        
    private:
        int64_t _last_landmark_ts = 0;
        bool _hasFace = false;
        NormalizedLandmarkList _lastLandmark;
        NormalizedLandmarkList _emptyLandmark;
        FaceMeshModuleIMP *_imp = nullptr;
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
        virtual bool init(long glContext, void *binaryData, int size) override;

        virtual void startModule() override;

        virtual void stopModule() override;

        virtual void initLut(int width, int height, void *lutData, int size) override;

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
        
        bool getSegmentation() override {
            return _segEnable;
        }
        
        void setSegmentationEnable(bool segEnable) override;

        void setSegmentationMask(mediapipe::GpuBuffer segMask);
        
        OpipeDispatch* currentDispatch() {
            return _dispatch.get();
        }

        void runInContextSync(std::function<void()> func) override {
            _dispatch->runSync(func);
        }

        virtual void setInputSource(Source *source) override;

        Filter* getOutputFilter() override;

    private:
        std::unique_ptr<OpipeDispatch> _dispatch;
        std::unique_ptr<OlaGraph> _graph;
        Context *_context = nullptr;
        bool _isInit = false;
        bool _segEnable = false;
        NormalizedLandmarkList _lastLandmark;
        std::shared_ptr<FaceMeshCallFrameDelegate> _delegate;
        FaceMeshBeautyRender *_render = nullptr;
        OlaContext *_olaContext = nullptr;
        Timestamp _lastTs = Timestamp::Unset();
        OMat _omat;
#if TestTemplateFace
        SourceImage *_templateFace = nullptr;
#endif
    };
}
#endif
