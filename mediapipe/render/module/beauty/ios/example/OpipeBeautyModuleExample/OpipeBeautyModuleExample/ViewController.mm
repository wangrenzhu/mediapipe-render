//
//  ViewController.m
//  OpipeBeautyModuleExample
//
//  Created by 王韧竹 on 2022/7/21.
//

#import "ViewController.h"
#import <OlaCameraFramework/OlaMTLCameraRenderView.h>
#import <OlaFaceUnityFramework/OlaFaceUnity.h>

@interface ViewController ()
<AVCaptureVideoDataOutputSampleBufferDelegate,
OlaMTLCameraRenderViewDelegate,
AVCaptureAudioDataOutputSampleBufferDelegate>
{
    CFAbsoluteTime _startRunTime;
    CFAbsoluteTime _currentRunTIme;
}

/**
 相机当前位置
 @return 0:后置 1：前置
 */
- (int)devicePosition;

/**
 切换前后摄像头
 */
- (void)rotateCamera;

- (void)startCapture;
- (void)stopCapture;

- (void)pauseCapture;
- (void)resumeCapture;

@property (nonatomic, strong) AVCaptureSession *captureSession;
@property (nonatomic, retain) AVCaptureDevice *captureDevice;
@property (nonatomic, strong) AVCaptureDeviceInput *videoInput;
@property (nonatomic, strong) AVCaptureVideoDataOutput *videoOutput;
@property (nonatomic, strong) AVCaptureAudioDataOutput *audioOutput;
@property (nonatomic, assign) CGSize cameraSize;
@property (nonatomic, assign) int pixelFormatType;
@property (nonatomic, assign) CGSize previewSize;

@property (nonatomic, assign) BOOL isCapturePaused;
@property (nonatomic, strong) OlaMTLCameraRenderView *renderView;


@end

@implementation ViewController

- (void)dealloc {
    [[OlaFaceUnity sharedInstance] dispose];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.pixelFormatType = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
    _cameraSize = CGSizeMake(1280, 720);
    if (abs(_currentRunTIme - 0) < 0.0001) {
        _startRunTime = CFAbsoluteTimeGetCurrent();
        _currentRunTIme = 0.;
    }
    [OlaFaceUnity sharedInstance].useGLRender = NO;
    [self setupSession];
    [[OlaFaceUnity sharedInstance] initModule];
    [[OlaFaceUnity sharedInstance] resume];
    
}

- (void)viewDidLayoutSubviews
{
    [super viewDidLayoutSubviews];
    if (CGSizeEqualToSize(self.previewSize, self.view.bounds.size)) {
        return;
    }
    _previewSize = self.view.bounds.size;
    [self setupRenderView];
    [self.renderView setNeedFlip:YES];
    
}

- (void)viewWillDisappear:(BOOL)animated {
    [[OlaFaceUnity sharedInstance] suspend];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    [self startCapture];
    [[OlaFaceUnity sharedInstance] resume];
    [OlaFaceUnity sharedInstance].whiten = 0.0;
    [OlaFaceUnity sharedInstance].smooth = 0.0;
}

- (void)setupSession {
    self.captureSession = [[AVCaptureSession alloc] init];
    [self.captureSession beginConfiguration];
    
    // 设置换面尺寸
    [self.captureSession setSessionPreset:AVCaptureSessionPreset1280x720];
    // 设置输入设备
    AVCaptureDevice *inputCamera = nil;
    NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for (AVCaptureDevice *device in devices) {
        if ([device position] == AVCaptureDevicePositionFront) {
            inputCamera = device;
            self.captureDevice = device;
        }
    }
    
    if (!inputCamera) {
        return;
    }
    
    NSError *error = nil;
    _videoInput = [[AVCaptureDeviceInput alloc] initWithDevice:inputCamera error:&error];
    if ([self.captureSession canAddInput:_videoInput]) {
        [self.captureSession addInput:_videoInput];
    }
    
    // 设置输出数据
    _videoOutput = [[AVCaptureVideoDataOutput alloc] init];
    [_videoOutput setVideoSettings:[NSDictionary dictionaryWithObject:[NSNumber numberWithInt:self.pixelFormatType]
                                                               forKey:(id)kCVPixelBufferPixelFormatTypeKey]];
    [_videoOutput setSampleBufferDelegate:self queue:dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0)];
    
    if ([self.captureSession canAddOutput:_videoOutput]) {
        [self.captureSession addOutput:_videoOutput];
    }
    
    //[self setupAudioCapture]; // 音频
    
    [self.captureSession commitConfiguration];
    
    NSDictionary* outputSettings = [_videoOutput videoSettings];
    for(AVCaptureDeviceFormat *vFormat in [self.captureDevice formats]) {
        CMFormatDescriptionRef description= vFormat.formatDescription;
        float maxrate = ((AVFrameRateRange*)[vFormat.videoSupportedFrameRateRanges objectAtIndex:0]).maxFrameRate;
        
        CMVideoDimensions dimensions = CMVideoFormatDescriptionGetDimensions(description);
        FourCharCode formatType = CMFormatDescriptionGetMediaSubType(description);
        if(maxrate == 30 && formatType == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange &&
           dimensions.width ==[[outputSettings objectForKey:@"Width"]  intValue] &&
           dimensions.height ==[[outputSettings objectForKey:@"Height"]  intValue]) {
            if (YES == [self.captureDevice lockForConfiguration:NULL] ) {
                self.captureDevice.activeFormat = vFormat;
                [self.captureDevice setActiveVideoMinFrameDuration:CMTimeMake(1,30)];
                [self.captureDevice setActiveVideoMaxFrameDuration:CMTimeMake(1,30)];
                [self.captureDevice unlockForConfiguration];
            }
        }
    }
}

- (void)setupRenderView {
    if(!self.renderView){
        _renderView = [[OlaMTLCameraRenderView alloc] initWithFrame:self.view.bounds shareContext:[[OlaFaceUnity sharedInstance] currentContext]];
        _renderView.cameraDelegate = self;
        _renderView.preferredFramesPerSecond = 30;
        [self.renderView setBackgroundColor:[UIColor colorWithRed:0.9f green:0.9f blue:0.9f alpha:1.0f]];
        [self.view addSubview:self.renderView];
        [self.view sendSubviewToBack:self.renderView];
    }
}

#pragma mark - <AVCaptureVideoDataOutputSampleBufferDelegate>
- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection {
    if (self.isCapturePaused || !self.captureSession.isRunning) {
        return;
    }
    
    if (captureOutput == _videoOutput) {
        [self.renderView cameraSampleBufferArrive:sampleBuffer];
    }
}

- (int)devicePosition
{
    AVCaptureDevicePosition currentCameraPosition = [[self.videoInput device] position];
    if (currentCameraPosition == AVCaptureDevicePositionBack) {
        return 0;
    } else {
        return 1;
    }
}

- (void)rotateCamera {
    AVCaptureDevicePosition currentCameraPosition = [[self.videoInput device] position];
    if (currentCameraPosition == AVCaptureDevicePositionBack) {
        currentCameraPosition = AVCaptureDevicePositionFront;
    } else {
        currentCameraPosition = AVCaptureDevicePositionBack;
    }
    
    AVCaptureDevice *backFacingCamera = nil;
    NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for (AVCaptureDevice *device in devices) {
        if ([device position] == currentCameraPosition) {
            backFacingCamera = device;
        }
    }
    
    NSError *error;
    AVCaptureDeviceInput *newVideoInput = [[AVCaptureDeviceInput alloc] initWithDevice:backFacingCamera error:&error];
    if (newVideoInput != nil) {
        [self.captureSession beginConfiguration];
        [self.captureSession setSessionPreset:AVCaptureSessionPreset1280x720];
        
        [self.captureSession removeInput:self.videoInput];
        if ([self.captureSession canAddInput:newVideoInput]) {
            [self.captureSession addInput:newVideoInput];
            self.videoInput = newVideoInput;
        } else {
            [self.captureSession addInput:self.videoInput];
        }
        [self.captureSession commitConfiguration];
    }
}

- (void)startCapture {
    self.isCapturePaused = NO;
    if (self.captureSession && ![self.captureSession isRunning]) {
        [self.captureSession startRunning];
    }
}

- (IBAction)switchSegmentation:(UISwitch *)sender {
    [OlaFaceUnity sharedInstance].useSegmentation = sender.isOn;
}

- (void)stopCapture {
    self.isCapturePaused = YES;
    if (self.captureSession) {
        [self.videoOutput setSampleBufferDelegate:nil queue:nil];
        
        [self.captureSession stopRunning];
        [self.captureSession removeInput:self.videoInput];
        [self.captureSession removeOutput:self.videoOutput];
        [self.captureSession removeOutput:self.audioOutput];
        
        self.videoOutput = nil;
        self.videoInput = nil;
        self.captureSession = nil;
        self.captureDevice = nil;
    }
}

- (void)pauseCapture {
    self.isCapturePaused = YES;
}

- (void)resumeCapture {
    self.isCapturePaused = NO;
    if (!self.captureSession.isRunning) {
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
            if(!self.captureSession.isRunning){
                [self.captureSession startRunning];
            }
        });
    }
}

- (void)bgraCameraTextureReady:(OlaShareTexture *)texture
               onScreenTexture:(OlaShareTexture *)onScreenTexture
                     frameTime:(NSTimeInterval)frameTime
{
    
    [[OlaFaceUnity sharedInstance] processVideoFrame:texture.renderTarget timeStamp:frameTime];
}

- (IOSurfaceID)externalRender:(NSTimeInterval)frameTime
                targetTexture:(OlaShareTexture *)targetTexture
                commandBuffer:(id<MTLCommandBuffer>)buffer
{
    //    [[OlaFaceUnity sharedInstance] processVideoFrame:targetTexture.renderTarget timeStamp:frameTime];
    FaceTextureInfo inputTexture;
    inputTexture.width = targetTexture.size.width;
    inputTexture.height = targetTexture.size.height;
    inputTexture.textureId = targetTexture.openGLTexture;
    inputTexture.ioSurfaceId = targetTexture.surfaceID;
    inputTexture.frameTime = frameTime;
    FaceTextureInfo result = [[OlaFaceUnity sharedInstance] render:inputTexture];
    return result.ioSurfaceId;
}

- (void)yuvTextureReady:(OlaShareTexture *)yTexture uvTexture:(OlaShareTexture *)uvTexture
{
    
}

- (void)draw:(NSTimeInterval)frameTime
{
    
}

- (IBAction)beautyChanged:(UISlider *)sender
{
    if (sender.tag == 0) {
        [OlaFaceUnity sharedInstance].whiten = sender.value;
        [OlaFaceUnity sharedInstance].smooth = sender.value;
    } else if (sender.tag == 1) {
        [OlaFaceUnity sharedInstance].slim = sender.value;
    } else if (sender.tag == 2) {
        [OlaFaceUnity sharedInstance].eyeFactor = sender.value;
    } else if (sender.tag == 3) {
        [OlaFaceUnity sharedInstance].nose = sender.value;
    }
}

@end
