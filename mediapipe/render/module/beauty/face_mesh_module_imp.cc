#include "face_mesh_module_imp.h"
#include "mediapipe/render/core/Context.hpp"
#include "mediapipe/render/core/OpipeDispatch.hpp"
#include "mediapipe/render/core/math/vec2.hpp"
#include "mediapipe/gpu/gl_texture_buffer.h"
#include "mediapipe/gpu/gl_base.h"
#include "mediapipe/gpu/gpu_shared_data_internal.h"

#if defined(__APPLE__)
#include "mediapipe/render/core/CVFramebuffer.hpp"
#endif

#define PRESTREAMING_FRAMETIME 1

static const char *kNumFacesInputSidePacket = "num_faces";
static const char *kVFlipInputSidePacket = "vflip";
static const char *kLandmarksOutputStream = "multi_face_landmarks";
static const char *kDetectionsOutputStream = "face_detections";
static const char *kOutputVideo = "output_video";
static const char *kInputVideo = "pre_stream_landmark_video";
static const char *kSegmentInputVideo = "segment_input_video";
static const char *kSegmentation = "segment_video";
static const char *kUseSegmentation = "use_segmentation";
static const char *kUseLandmarks = "use_landmarks";

namespace Opipe
{
    time_t getTimeStampValue()
    {
            std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds> tp =
                    std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now());
            auto tmp = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch());
            time_t timestamp = tmp.count();
            return timestamp;
    }

    FaceMeshCallFrameDelegate::FaceMeshCallFrameDelegate()
    {
    }

    FaceMeshCallFrameDelegate::~FaceMeshCallFrameDelegate()
    {
    }

    void FaceMeshCallFrameDelegate::outputPacket(OlaGraph *graph, const mediapipe::Packet &packet,
                                                 MPPPacketType packetType,
                                                 const std::string &streamName)
    {
        if (_imp == nullptr)
        {
            return;
        }
        LOG(INFO) << "######  FaceMeshCallFrameDelegate::outputPacket:" << streamName << " packetType:" << packetType;
        _imp->currentDispatch()->runAsync([&, packetType, streamName, packet, graph] {

            int64_t currentTime = getTimeStampValue();

            if (streamName == kLandmarksOutputStream)
            {
                _last_landmark_ts = packet.Timestamp().Value();
                _hasFace = true;
                const auto &multi_face_landmarks = packet.Get<std::vector<::mediapipe::NormalizedLandmarkList>>();
                _lastLandmark = multi_face_landmarks[0];
            }
            #if defined(__APPLE__)
            int64_t distance = 1000000;
            #else
            int64_t distance = 1000;
            #endif

            if (streamName == kOutputVideo && (packet.Timestamp().Value() - _last_landmark_ts) > distance)
            {
                _hasFace = false;
                _last_landmark_ts = 0; //输出过一次的时间戳 不再输出
            }

            if (_hasFace)
            {
                // 人脸识别数据
                _imp->setLandmark(_lastLandmark, packet.Timestamp().Value());
            }
            else
            {
                _imp->setLandmark(_emptyLandmark, packet.Timestamp().Value());
            }
         
            
            if (streamName == kSegmentation && _imp->getSegmentation()) {
                // 人脸分割的数据 这里需要重写
                LOG(INFO) << "######  FaceMeshCallFrameDelegate kSegmentation:" << streamName << " packetType:" << packetType;
                SourceCamera *cameraSource = _imp->getOutputSource();
                
#if defined(__APPLE__)
                if (packetType != MPPPacketTypePixelBuffer) {
                    return;
                }
                const mediapipe::GpuBuffer& video = packet.Get<GpuBuffer>();
                CVPixelBufferRef pixelbuffer = mediapipe::GetCVPixelBufferRef(video);
                IOSurfaceRef ioSurface = CVPixelBufferGetIOSurface(pixelbuffer);
                IOSurfaceLock(ioSurface, kIOSurfaceLockReadOnly, 0);
                int ioSurfaceId = IOSurfaceGetID(ioSurface);
                _imp->getRender()->updateSegmentMarkIOSurfaceId(ioSurfaceId, video.width(), video.height());
                
                LOG(INFO) << "###### kSegmentation ioSurfaceId:" << ioSurfaceId;
#else
                if (packetType != MPPPacketTypeGpuBuffer) {
                    return;
                }
                const mediapipe::GpuBuffer& video = packet.Get<GpuBuffer>();
                mediapipe::GlTextureBufferSharedPtr ptr = video.internal_storage<mediapipe::GlTextureBuffer>();
                
                ptr->WaitUntilComplete();
                int textureId = ptr->name();
                LOG(INFO) << "###### FaceMeshCallFrameDelegate::textureId:" << textureId;
                _imp->getRender()->updateSegmentMarkTexture(textureId, video.width(), video.height());
#endif
                OlaCameraSource *inputSource = _imp->getInputSource();
                Framebuffer* framebuffer = inputSource->getRenderFramebuffer();
                if (framebuffer) {
                    _imp->getRender()->setSegmentEnable(true);
                    cameraSource->setRenderTexture(framebuffer->getTexture(), framebuffer->getWidth(), framebuffer->getHeight());
                    cameraSource->updateTargets(packet.Timestamp().Value());
                }

            }

            if (streamName == kOutputVideo && !_imp->getSegmentation()) {
                if (_imp->getOutputSource()) {
                    SourceCamera *cameraSource = _imp->getOutputSource();
                    
#if defined(__APPLE__)
                    if (packetType != MPPPacketTypePixelBuffer) {
                        return;
                    }
                    
                    
#else
                    if (packetType != MPPPacketTypeGpuBuffer) {
                        return;
                    }
#endif

                    OlaCameraSource *inputSource = _imp->getInputSource();
                    Framebuffer* framebuffer = inputSource->getRenderFramebuffer();
                    if (framebuffer) {
                        LOG(INFO) << "###### getRenderFramebuffer FaceMeshCallFrameDelegate::textureId:" << framebuffer->getTexture();
                        cameraSource->setRenderTexture(framebuffer->getTexture(), framebuffer->getWidth(), framebuffer->getHeight());
                        cameraSource->updateTargets(packet.Timestamp().Value());
                    }
                }
            }
 
#ifdef PRESTREAMING_FRAMETIME
            int64_t renderSubmitTimeStamp = getTimeStampValue();
            float renderSubmitDuration = (float)(renderSubmitTimeStamp - currentTime) / 1000.0;
            
            if (renderSubmitDuration > _profile.maxRenderDuration) {
                _profile.maxRenderDuration = renderSubmitDuration;
            }
            
            if (renderSubmitDuration < _profile.minRenderDuration) {
                _profile.minRenderDuration = renderSubmitDuration;
            }
            
            _profile.renderCount = _profile.renderCount + 1;
            _profile.avgRenderTime = renderSubmitDuration / (float)_profile.renderCount +
            _profile.avgRenderTime * ((float)(_profile.renderCount - 1) /
                                            (float)_profile.renderCount);
        
#endif

        });
    }

    FaceMeshModuleIMP::FaceMeshModuleIMP()
    {
    }

    FaceMeshModuleIMP::~FaceMeshModuleIMP()
    {
        _delegate->attach(nullptr);
        _delegate = 0;

        if (_inputSource)
        {
            _dispatch->runSync([&] {
                _inputSource->removeAllTargets();
                delete _inputSource;
                _inputSource = nullptr; 
            }, Context::IOContext);
        }

        if (_olaContext)
        {
            delete _olaContext;
            _olaContext = nullptr;
        }

        _graph = nullptr;

        if (_render)
        {
            _dispatch->runSync([&] {
                _outputSource->removeAllTargets();
                delete _render;
                _render = nullptr;
                if (_outputSource) {
                    delete _outputSource;
                    _outputSource = nullptr;
                } 
            });
        }
        delete _context;
        _context = nullptr;
    }

    void FaceMeshModuleIMP::suspend()
    {
        if (_render)
        {
            _render->suspend();
        }
    }

    void FaceMeshModuleIMP::resume()
    {
        if (_render)
        {
            _render->resume();
        }
    }

    void FaceMeshModuleIMP::initLut(OMat &mat, OMat &grayMat)
    {
        _omat = std::move(mat);
        _grayMat = std::move(grayMat);
    }

    bool FaceMeshModuleIMP::init(GLThreadDispatch *glDispatch, long glcontext, void *binaryData, 
                                 int size, bool useBeautyV2)
    {
        _useBeautyV2 = useBeautyV2;
        _delegate = std::make_shared<FaceMeshCallFrameDelegate>();
        _delegate->attach(this);
        mediapipe::CalculatorGraphConfig config;
        config.ParseFromArray(binaryData, size);
        _olaContext = new OlaContext();
        _context = _olaContext->glContext();
#if defined(__ANDROID__)
        _context->initEGLContext((EGLContext)glcontext);
        LOG(INFO) << "###### glThreadDispatch std::this_thread::get_id()  " << std::this_thread::get_id();
        _dispatch = std::make_unique<OpipeDispatch>(_context, this, std::move(glDispatch));
        _dispatch->setGLThreadDispatch(glDispatch);

        _graph = std::make_unique<OlaGraph>(config, (EGLContext)glcontext);
#else
        _dispatch = std::make_unique<OpipeDispatch>(_context, nullptr, nullptr);
        _graph = std::make_unique<OlaGraph>(config, _context->getEglContext());
        _landmarksEnable = true;
#endif

        _graph->setDelegate(_delegate);
        _graph->setSidePacket(mediapipe::MakePacket<int>(1), kNumFacesInputSidePacket);
        
        _graph->addFrameOutputStream(kLandmarksOutputStream, MPPPacketTypeRaw);
#if defined(__APPLE__)
        _graph->setSidePacket(mediapipe::MakePacket<bool>(false), kVFlipInputSidePacket);
        _graph->addFrameOutputStream(kOutputVideo, MPPPacketTypePixelBuffer);
#else
        _graph->setSidePacket(mediapipe::MakePacket<bool>(true), kVFlipInputSidePacket);
        _graph->addFrameOutputStream(kOutputVideo, MPPPacketTypeGpuBuffer);
#endif
        
#if defined(__APPLE__)
        _graph->addFrameOutputStream(kSegmentation, MPPPacketTypePixelBuffer);
#else
        _graph->addFrameOutputStream(kSegmentation, MPPPacketTypeGpuBuffer);
#endif
  
        if (_render == nullptr)
        {
            LOG(INFO) << "###### before init _render:" << _render;
#if defined(__APPLE__)
            _dispatch->runSync([&] {
#else
            _context->useAsCurrent();
#endif
            if (_render == nullptr)
            {
                if (_useBeautyV2)
                {
                     _render = new FaceMeshBeautyRender(_context, _omat, _grayMat);
                }
                else
                {
                     _render = new FaceMeshBeautyRender(_context, _omat);
                }
               
                _outputSource = SourceCamera::create(_context); // mediapipe 的输出source
                _render->setInputSource(_outputSource);         //作为渲染管线的源头
                LOG(INFO) << "###### after init _render:" << _render;
            }
            LOG(INFO) << "###### before init _inputSource:" << _inputSource;
#if defined(__APPLE__)
            });
#endif
            _dispatch->runSync([&]
                               {
#if defined(__APPLE__)
                    _inputSource = new OlaCameraSource(_context, Opipe::SourceCamera::SourceType_YUV420SP);
#else
                    _inputSource = new OlaCameraSource(_context, Opipe::SourceCamera::SourceType_RGBA);
#endif
                    LOG(INFO) << "###### after init _inputSource" << _inputSource; 
            }, Context::IOContext);
        }
        _isInit = true;
        return true;
    }

    void FaceMeshModuleIMP::setLandmark(NormalizedLandmarkList landmark, int64_t timeStamp)
    {

        _lastLandmark = std::move(landmark);

        if (_lastLandmark.landmark_size() == 0)
        {
            LOG(INFO) << "hasFace 没有检测到人脸";
        }
        else
        {
            LOG(INFO) << "hasFace 检测到人脸输出";
        }

        std::vector<Vec2> facePoints;
        if (_lastLandmark.landmark_size() > 0)
        {
            LOG(INFO) << "检测到人脸输出:" << _lastLandmark.landmark_size();
            for (int i = 0; i < _lastLandmark.landmark_size(); i++)
            {
                #if defined(__APPLE__)
                facePoints.emplace_back(_lastLandmark.landmark(i).x(), _lastLandmark.landmark(i).y());
                #else
                facePoints.emplace_back(_lastLandmark.landmark(i).x(), 1.0 - _lastLandmark.landmark(i).y());
                #endif
            }
            LOG(INFO) << "@@@facePoint i:" << 0 << " x:" << _lastLandmark.landmark(152).x() << " y:" << _lastLandmark.landmark(152).y();
            LOG(INFO) << "检测到人脸输完毕:" << _lastLandmark.landmark_size();
        }
        _render->setFacePoints(facePoints);
    }

    void FaceMeshModuleIMP::startModule()
    {
        if (!_isInit)
        {
            return;
        }
        LOG(INFO) << "###### before start";
        _isInit = _graph->start();
        _graph->setUseVideoOutput(false);
        LOG(INFO) << "###### after start" << _isInit;
    }

    void FaceMeshModuleIMP::stopModule()
    {
        if (!_isInit)
        {
            return;
        }
        _graph->setDelegate(nullptr);
        _graph->cancel();
        _graph->closeAllInputStreams();
        _graph->waitUntilDone();
    }

    void FaceMeshModuleIMP::setSegmentationEnable(bool segEnable)
    {
        _segEnable = segEnable;
        _dispatch->runAsync([&] {
            if (!segEnable) {
                _render->setSegmentEnable(false);
            }
        });
    }
    
    void FaceMeshModuleIMP::setLandmarksEnable(bool landmarksEnable)
    {
        _landmarksEnable = landmarksEnable;
        _dispatch->runAsync([&] {
            
            _render->setFaceLandmarkEnable(_landmarksEnable);
        });
    }

#if defined(__APPLE__)
    void FaceMeshModuleIMP::processVideoFrame(CVPixelBufferRef pixelbuffer,
                                              int64_t timeStamp)
    {
        if (!_isInit)
        {
            return;
        }
        _dispatch->runSync([&]
                           {
            CVPixelBufferLockBaseAddress(pixelbuffer, 0);
            
            int width = (int)CVPixelBufferGetWidth(pixelbuffer);
            int height = (int)CVPixelBufferGetHeight(pixelbuffer);
            
            _inputSource->setFrameData(width,
                                       height,
                                       CVPixelBufferGetBaseAddressOfPlane(pixelbuffer, 0),
                                       GL_RGBA,
                                       -1,
                                       RotationMode::RotateRightFlipVertical,
                                       Opipe::SourceCamera::SourceType_YUV420SP,
                                       CVPixelBufferGetBaseAddressOfPlane(pixelbuffer, 1));
            CVPixelBufferUnlockBaseAddress(pixelbuffer, 0);
            _inputSource->updateTargets(timeStamp);
            CVFramebuffer *faceFramebuffer = dynamic_cast<CVFramebuffer *>(_inputSource->getFaceFramebuffer());
            Timestamp ts(timeStamp * 1000);
            
            _graph->sendPacket(mediapipe::MakePacket<bool>(_segEnable).At(ts), kUseSegmentation);
            _graph->sendPacket(mediapipe::MakePacket<bool>(_landmarksEnable).At(ts), kUseLandmarks);
            
            if (faceFramebuffer && faceFramebuffer->renderTarget) {
                CVPixelBufferLockBaseAddress(faceFramebuffer->renderTarget, 0);
                _graph->sendPixelBuffer(faceFramebuffer->renderTarget, kInputVideo,
                                        MPPPacketTypePixelBuffer,
                                        ts);
                CVPixelBufferUnlockBaseAddress(faceFramebuffer->renderTarget, 0);
            }

            if (_segEnable)
            {
                CVFramebuffer *segFramebuffer = dynamic_cast<CVFramebuffer *>(_inputSource->getSegmentationFramebuffer());
                if (segFramebuffer && segFramebuffer->renderTarget) {
                    CVPixelBufferLockBaseAddress(segFramebuffer->renderTarget, 0);
                    _graph->sendPixelBuffer(segFramebuffer->renderTarget, kSegmentInputVideo,
                                            MPPPacketTypePixelBuffer,
                                            ts);
                    CVPixelBufferUnlockBaseAddress(segFramebuffer->renderTarget, 0);
                }
            }
        }, Context::IOContext);
    }

    void FaceMeshModuleIMP::setSegmentationBackground(UIImage *image)
    {

    }

#else
    void FaceMeshModuleIMP::setSegmentationBackground(OMat background)
    {

    }
#endif

    void FaceMeshModuleIMP::processVideoFrame(unsigned char *pixelbuffer,
                                              int size,
                                              int width,
                                              int height,
                                              int64_t timeStamp)
    {
        // 这里暂时不实现
        return;
    }

    void FaceMeshModuleIMP::processVideoFrame(int textureId, int width, int height, int64_t timeStamp)
    {
         Timestamp ts(timeStamp);
        _inputSource->setRenderTexture(textureId, width, height);
        _inputSource->updateTargets(timeStamp);
        _dispatch->runSync([&] {
            if (!_isInit)
            {
                return;
            }

            Framebuffer *faceFramebuffer = _inputSource->getFaceFramebuffer();

            _graph->sendPacket(mediapipe::MakePacket<bool>(_segEnable).At(ts), kUseSegmentation);
            _graph->sendPacket(mediapipe::MakePacket<bool>(_landmarksEnable).At(ts), kUseLandmarks);
            if (faceFramebuffer)
            {
                _graph->sendPacket(faceFramebuffer->getTexture(), width, height, kInputVideo, timeStamp);
            }
            LOG(INFO) << "processVideoFrame:" << "before getSegmentationFramebuffer " << "_landmarksEnable:" << _landmarksEnable;
            Framebuffer *segFramebuffer = _inputSource->getSegmentationFramebuffer();

            if (_segEnable && segFramebuffer)
            {
                _graph->sendPacket(segFramebuffer->getTexture(), width, height, kSegmentInputVideo, timeStamp);
            }
        }, Context::IOContext);
    }

    TextureInfo FaceMeshModuleIMP::renderTexture(TextureInfo inputTexture)
    {
        TextureInfo textureInfo;

        if (!_isInit)
        {
            return inputTexture;
        }
        LOG(INFO) << "###### before FaceMeshModuleIMP renderTexture:" << inputTexture.textureId;
        textureInfo = _render->outputRenderTexture(inputTexture);
        LOG(INFO) << "###### after FaceMeshModuleIMP renderTexture:" << textureInfo.textureId;

        return textureInfo;
    }

    Filter *FaceMeshModuleIMP::getOutputFilter()
    {
        return _render->outputFilter();
    }

    void FaceMeshModuleIMP::setInputSource(Source *source)
    {
        _render->setInputSource(source);
    }
}
