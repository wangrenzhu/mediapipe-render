#include "face_mesh_module_imp.h"
#include "mediapipe/render/core/Context.hpp"
#include "mediapipe/render/core/OpipeDispatch.hpp"
#include "mediapipe/render/core/math/vec2.hpp"

#if TestTemplateFace
#include "mediapipe/render/core/CVFramebuffer.hpp"
#import <UIKit/UIKit.h>
#else
#if defined(__APPLE__)
#include "mediapipe/render/core/CVFramebuffer.hpp"
#endif
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
//        if (streamName == kSegmentation) {
//            _imp->currentDispatch()->runSync([&] {
//                IOSurfaceRef surface = CVPixelBufferGetIOSurface(pixelbuffer);
//                IOSurfaceID surfaceId = IOSurfaceGetID(surface);
//
//                CVFramebuffer *framebuffer = (CVFramebuffer *)_imp->getOutputFilter()->getFramebuffer();
//                CVPixelBufferRef pbuffer = framebuffer->renderTarget;
//                Log("Opipe", "streamName %s timeStamp:%ld iosurfaceid:%d width:%d originWidth:%d", streamName.c_str(), timestamp, surfaceId,
//                    (int)IOSurfaceGetWidth(surface), (int)CVPixelBufferGetWidth(pbuffer));
//
//            });
//        }
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
                
                _imp->setLandmark(_lastLandmark, packet.Timestamp().Value());
            } else {
                _imp->setLandmark(_emptyLandmark, packet.Timestamp().Value());
            }
            
           
        }, Context::IOContext);
        if (streamName == kSegmentation) {
//                _imp->currentDispatch()->runSync([&] {
                // 人脸分割的数据
                const auto& image = packet.Get<Image>();
                if (image.UsesGpu()) {
                    auto gpubuffer = image.GetGpuBuffer();
                    _imp->setSegmentationMask(gpubuffer);
                }
//                });
        }
    }

    void FaceMeshCallFrameDelegate::outputPacket(OlaGraph *graph, const mediapipe::Packet &packet,
                                                 MPPPacketType packetType, const std::string &streamName)
    {
#if defined(__APPLE__)
//        if (streamName == kLandmarksOutputStream) {
//          if (packet.IsEmpty()) {
//            NSLog(@"[TS:%lld] No face landmarks", packet.Timestamp().Value());
//            return;
//          }
//          const auto& multi_face_landmarks = packet.Get<std::vector<::mediapipe::NormalizedLandmarkList>>();
//          NSLog(@"[TS:%lld] Number of face instances with landmarks: %lu", packet.Timestamp().Value(),
//                multi_face_landmarks.size());
//          for (int face_index = 0; face_index < multi_face_landmarks.size(); ++face_index) {
//            const auto& landmarks = multi_face_landmarks[face_index];
//            NSLog(@"\tNumber of landmarks for face[%d]: %d", face_index, landmarks.landmark_size());
//            for (int i = 0; i < landmarks.landmark_size(); ++i) {
//              NSLog(@"\t\tLandmark[%d]: (%f, %f, %f)", i, landmarks.landmark(i).x(),
//                    landmarks.landmark(i).y(), landmarks.landmark(i).z());
//            }
//          }
//        } else if (streamName == kDetectionsOutputStream) {
//            if (packet.IsEmpty()) {
//              NSLog(@"[TS:%lld] No face detections", packet.Timestamp().Value());
//              return;
//            }
//            const auto& face_detections = packet.Get<std::vector<::mediapipe::Detection>>();
//            NSLog(@"[TS:%lld] Number of face instances with detections: %lu", packet.Timestamp().Value(),
//                  face_detections.size());
//
//        }
#endif
    }

    FaceMeshModuleIMP::FaceMeshModuleIMP()
    {
    }

    FaceMeshModuleIMP::~FaceMeshModuleIMP()
    {
        _delegate->attach(nullptr);
        _delegate = 0;
        
        if (_olaContext) {
            delete _olaContext;
            _olaContext = nullptr;
        }

        _graph = nullptr;

        if (_render) {
            _dispatch->runSync([&] {
                delete _render;
                _render = nullptr;
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
                    _render = new FaceMeshBeautyRender(_context, _omat.width, _omat.height, _omat.data);
#if TestTemplateFace
                    UIImage *image = [UIImage imageNamed:@"templateFace"];
                    
                    _templateFace = SourceImage::create(_context, image);
        
#endif

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
//            _graph->cosumeFrame();
//            _graph->closeAllInputStreams();
            Log("FaceMeshModule", "检测到人脸输出");
        }
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
        Timestamp ts(timeStamp * 1000);
        _graph->sendPacket(mediapipe::MakePacket<bool>(_segEnable).At(ts), kUseSegmentation);
#if TestTemplateFace
        auto *framebuffer = dynamic_cast<CVFramebuffer *>(_templateFace->getFramebuffer());
        CVPixelBufferRef renderTarget = framebuffer->renderTarget;
        framebuffer->lockAddress();
        _graph->sendPixelBuffer(renderTarget, kInputVideo,
        MPPPacketTypePixelBuffer,
        ts);
        framebuffer->unlockAddress();
#else
        CVPixelBufferLockBaseAddress(pixelbuffer, 0);
        _graph->sendPixelBuffer(pixelbuffer, kInputVideo,
        MPPPacketTypePixelBuffer,
        ts);
        CVPixelBufferUnlockBaseAddress(pixelbuffer, 0);
#endif
        _dispatch->runSync([&] {
            std::vector<Vec2> facePoints;
            if (_lastLandmark.landmark_size() > 0) {
                Log("FaceMeshModule", "检测到人脸输出");
                for (int i = 0; i < _lastLandmark.landmark_size(); i++) {
                    facePoints.emplace_back( _lastLandmark.landmark(i).x(), _lastLandmark.landmark(i).y());
                }
                Log("FaceMeshModule", "检测到人脸输完毕");
            }
            _render->setFacePoints(facePoints);
        }, Context::IOContext);
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
