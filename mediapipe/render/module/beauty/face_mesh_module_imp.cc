#include "face_mesh_module_imp.h"
#include "mediapipe/render/core/Context.hpp"
#include "mediapipe/render/core/OpipeDispatch.hpp"
#include "mediapipe/render/core/math/vec2.hpp"

#if defined(__APPLE__)
#include "mediapipe/render/core/CVFramebuffer.hpp"
#endif

static const char* kNumFacesInputSidePacket = "num_faces";
static const char* kLandmarksOutputStream = "multi_face_landmarks";
static const char* kDetectionsOutputStream = "face_detections";
static const char* kOutputVideo = "output_video";
static const char* kInputVideo = "input_video";
static const char* kSegmentation = "filtered_segmentation_mask";
static const char* kUseSegmentation = "use_segmentation";

namespace Opipe
{

    FaceMeshCallFrameDelegate::FaceMeshCallFrameDelegate()
    {
    }

    FaceMeshCallFrameDelegate::~FaceMeshCallFrameDelegate()
    {
    }
#if defined(__APPLE__)
    void FaceMeshCallFrameDelegate::outputPixelbuffer(OlaGraph *graph, CVPixelBufferRef pixelbuffer,
                                                      const std::string &streamName, int64_t timestamp)
    {

    }
#endif

    void FaceMeshCallFrameDelegate::outputPacket(OlaGraph *graph, const mediapipe::Packet &packet, const std::string &streamName) {

        if (_imp == nullptr) {
            return;
        }

        _imp->currentDispatch()->runSync([&] {
            if (streamName == kLandmarksOutputStream) {
                _last_landmark_ts = packet.Timestamp().Value();
                _hasFace = true;
                const auto& multi_face_landmarks = packet.Get<std::vector<::mediapipe::NormalizedLandmarkList>>();
                _lastLandmark = multi_face_landmarks[0];
            }
            Log("FaceMeshModule", "landmarkts:%ld", _last_landmark_ts);
            
            if (streamName == kOutputVideo && (packet.Timestamp().Value() - _last_landmark_ts) > 1000000) {
                _hasFace = false;
                _last_landmark_ts = 0; //输出过一次的时间戳 不再输出
            }
            
            if (_hasFace) {
                // 人脸识别数据
                _imp->setLandmark(_lastLandmark, packet.Timestamp().Value());
            } else {
                _imp->setLandmark(_emptyLandmark, packet.Timestamp().Value());
            }
            
            if (streamName == kSegmentation) {
                // 人脸分割的数据
                const auto& image = packet.Get<Image>();
                if (image.UsesGpu()) {
                    auto gpubuffer = image.GetGpuBuffer();
                    _imp->setSegmentationMask(gpubuffer);
                }
            }


            if (streamName == kOutputVideo) {
                // 这里是视频流 需要给 _outputSource
                const auto& video = packet.Get<GpuBuffer>();

                if (_imp->getOutputSource()) {
                    SourceCamera *cameraSource = _imp->getOutputSource();
#if defined(__APPLE__)

                    CVPixelBufferRef pixelbuffer = mediapipe::GetCVPixelBufferRef(video);
                    IOSurfaceRef ioSurface = CVPixelBufferGetIOSurface(pixelbuffer);
                    IOSurfaceLock(ioSurface, kIOSurfaceLockReadOnly, 0);
                    int ioSurfaceId = IOSurfaceGetID(ioSurface);
                    cameraSource->setIOSurfaceSource(ioSurfaceId, video.width(), video.height());
                    cameraSource->updateTargets(packet.Timestamp().Value());
                    IOSurfaceUnlock(ioSurface, kIOSurfaceLockReadOnly, 0);
#else

                    //上传数据到 texture 或者 共享纹理
#endif
                }
            }

        });

    }

    void FaceMeshCallFrameDelegate::outputPacket(OlaGraph *graph, const mediapipe::Packet &packet,
                                                 MPPPacketType packetType, const std::string &streamName)
    {

    }

    FaceMeshModuleIMP::FaceMeshModuleIMP()
    {
    }

    FaceMeshModuleIMP::~FaceMeshModuleIMP()
    {
        _delegate->attach(nullptr);
        _delegate = 0;
        
        if (_inputSource) {
            _dispatch->runSync([&] {
                _inputSource->removeAllTargets();
                delete _inputSource;
                _inputSource = nullptr;
            });
        }

        if (_olaContext) {
            delete _olaContext;
            _olaContext = nullptr;
        }

        _graph = nullptr;

        if (_render) {
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
        if (_render) {
            _render->suspend();
        }
    }

    void FaceMeshModuleIMP::resume()
    {
        if (_render) {
            _render->resume();
        }
    }

    void FaceMeshModuleIMP::initLut(int width, int height, void *lutData, int size){
        _omat = OMat(width, height,(char *)lutData);
     }

    bool FaceMeshModuleIMP::init(long glcontext, void *binaryData, int size)
    {
        _delegate = std::make_shared<FaceMeshCallFrameDelegate>();
        _delegate->attach(this);
        mediapipe::CalculatorGraphConfig config;
        config.ParseFromArray(binaryData, size);
        _olaContext = new OlaContext();
        _context = _olaContext->glContext();


#if defined(__ANDROID__)
        std::thread::id glThreadId = std::this_thread::get_id();

         auto *glThreadDispatch = new GLThreadDispatch(glThreadId, nullptr);

        _context->initEGLContext((EGLContext) glcontext);

        _dispatch = std::make_unique<OpipeDispatch>(_context, nullptr, glThreadDispatch);
        _graph = std::make_unique<OlaGraph>(config, (EGLContext)glcontext);
#else
        _dispatch = std::make_unique<OpipeDispatch>(_context, nullptr, nullptr);
        _graph = std::make_unique<OlaGraph>(config, _context->getEglContext());
#endif


        _graph->setDelegate(_delegate);
        _graph->setSidePacket(mediapipe::MakePacket<int>(1), kNumFacesInputSidePacket);
//        _graph->setSidePacket(mediapipe::MakePacket<bool>(false), kUseSegmentation);
        _graph->addFrameOutputStream(kLandmarksOutputStream, MPPPacketTypeRaw);
#if defined(__APPLE__)
        _graph->addFrameOutputStream(kOutputVideo, MPPPacketTypePixelBuffer);
#endif
        _graph->addFrameOutputStream(kOutputVideo, MPPPacketTypeGpubuffer);

        _graph->addFrameOutputStream(kSegmentation, MPPPacketTypeImage);
        _isInit = true;
        if (_render == nullptr) {
            _dispatch->runSync([&] {
                if (_render == nullptr) {
                    _inputSource = new OlaCameraSource(_context, Opipe::SourceCamera::SourceType_YUV420SP);
                    _render = new FaceMeshBeautyRender(_context, _omat.width, _omat.height, _omat.data);
                    _outputSource = SourceCamera::create(_context);
                    _render->setInputSource(_outputSource);
                }
            });
        }
        

        return true;
    }



    void FaceMeshModuleIMP::setLandmark(NormalizedLandmarkList landmark, int64_t timeStamp)
    {
        
        _lastLandmark = std::move(landmark);
        
        if (_lastLandmark.landmark_size() == 0) {
            Log("FaceMeshModule", "没有检测到人脸");
            
        } else {
            Log("FaceMeshModule", "检测到人脸输出");
        }

        std::vector<Vec2> facePoints;
        if (_lastLandmark.landmark_size() > 0) {
            Log("FaceMeshModule", "检测到人脸输出");
            for (int i = 0; i < _lastLandmark.landmark_size(); i++) {
                facePoints.emplace_back( _lastLandmark.landmark(i).x(), _lastLandmark.landmark(i).y());
            }
            Log("FaceMeshModule", "检测到人脸输完毕");
        }
        _render->setFacePoints(facePoints);
    }

    void FaceMeshModuleIMP::startModule()
    {
        if (!_isInit)
        {
            return;
        }
        _isInit = _graph->start();
        _graph->setUseVideoOutput(false);
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
        if (_render) {
            _render->setUseSegmentation(_segEnable);
        }
    }

    void FaceMeshModuleIMP::setSegmentationMask(mediapipe::GpuBuffer segMask)
    {
#if defined(__APPLE__)

        CVPixelBufferRef maskBuffer = mediapipe::GetCVPixelBufferRef(segMask);
        IOSurfaceRef maskSurface = CVPixelBufferGetIOSurface(maskBuffer);
        int width = (int)IOSurfaceGetWidth(maskSurface);
        int height = (int)IOSurfaceGetHeight(maskSurface);
        _context->useAsCurrent();
        CVFramebuffer *framebuffer = new CVFramebuffer(_context, width, height,
                                                       IOSurfaceGetID(maskSurface));
        _render->setSegmentationMask(std::move(framebuffer));

#else
        //这里需要好好写一下 怎么消费 mask
        //Android 需要异步渲染 需要waitOnGPU
#endif
    }

#if defined(__APPLE__)
    void FaceMeshModuleIMP::processVideoFrame(CVPixelBufferRef pixelbuffer,
                                              int64_t timeStamp)
    {
        if (!_isInit)
        {
            return;
        }
        _dispatch->runSync([&] {
            _context->useAsCurrent();
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
            CVFramebuffer *framebuffer = dynamic_cast<CVFramebuffer *>(_inputSource->getScaleFramebuffer());
            if (framebuffer && framebuffer->renderTarget) {
                Timestamp ts(timeStamp * 1000);
                _graph->sendPacket(mediapipe::MakePacket<bool>(_segEnable).At(ts), kUseSegmentation);
                CVPixelBufferLockBaseAddress(framebuffer->renderTarget, 0);
                _graph->sendPixelBuffer(framebuffer->renderTarget, kInputVideo,
                                        MPPPacketTypePixelBuffer,
                                        ts);
                CVPixelBufferUnlockBaseAddress(framebuffer->renderTarget, 0);
            }
        });
    }
    
    void FaceMeshModuleIMP::setSegmentationBackground(UIImage *image) {
        if (_render) {
            _context->useAsCurrent();
            SourceImage *sourceImage = SourceImage::create(_context, image);
            _render->setSegmentationBackground(std::move(sourceImage));
        }
    }

#else
    void FaceMeshModuleIMP::setSegmentationBackground(OMat background) {
        if (_render) {
            SourceImage *image = SourceImage::create(_context, background.width, background.height, background.data);
            _render->setSegmentationBackground(std::move(image));
        }
    }
#endif

    void FaceMeshModuleIMP::processVideoFrame(unsigned char *pixelbuffer,
                                              int size,
                                              int width,
                                              int height,
                                              int64_t timeStamp) {
        if (!_isInit) {
            return;
        }
        _graph->sendPacket(pixelbuffer, width, height, "input_video", timeStamp);

    }

    void FaceMeshModuleIMP::processVideoFrame(int textureId, int width, int height, int64_t timeStamp) {
        if (!_isInit) {
            return;
        }

        _graph->sendPacket(textureId, width, height, "input_video", timeStamp);

    }

    TextureInfo FaceMeshModuleIMP::renderTexture(TextureInfo inputTexture)
    {
        TextureInfo textureInfo;
        
        if (!_isInit)
        {
            return textureInfo;
        }
        if (_render == nullptr) {
            _dispatch->runSync([&] {
                if (_render == nullptr) {
                    _render = new FaceMeshBeautyRender(_context, _omat.width, _omat.height, _omat.data);
                }
            });
        }
        
        
        _dispatch->runSync([&] {
            
            _render->renderTexture(inputTexture);
        });
        
        textureInfo = _render->outputRenderTexture(inputTexture);

        return textureInfo;
    }

    Filter* FaceMeshModuleIMP::getOutputFilter()
    {
        return _render->outputFilter();
    }

    void FaceMeshModuleIMP::setInputSource(Source *source) {
        _dispatch->runSync([&] {
            _render->setInputSource(source);
        });
    }
}
