load("@build_bazel_rules_apple//apple:ios.bzl", "ios_framework", "ios_static_framework", "ios_application")
# bazel build --copt=-fembed-bitcode --apple_bitcode=embedded --config=ios_arm64
# 用上面这条指令build
# bazel build --copt=-fembed-bitcode --apple_bitcode=embedded --config=ios_arm64 mediapipe/render/module/beauty/ios/framework:OlaFaceUnityFramework


#  bazel build --config=ios_fat -c opt --cxxopt=--std=c++17 mediapipe/render/module/beauty/ios/framework:OlaFaceUnityFrameworkBeta   
#  bazel build --config=ios_fat -c opt --cxxopt=--std=c++17 mediapipe/render/module/beauty/ios/framework:OlaFaceUnityFramework      
ios_framework(
    name = "OlaFaceUnityFramework",
    hdrs = [
        "OlaFaceUnity.h",
        "OlaFURenderView.h",
    ],
    infoplists = ["Info.plist"],
    bundle_id = "com.ola.opipe.faceunity",
    families = ["iphone", "ipad"],
    minimum_os_version = "11.0",
    deps = [
         ":OlaFaceUnityLibrary",
         "@ios_opencv//:OpencvFramework",
    ],
)

objc_library(
    name = "OlaFaceUnityLibrary",
    hdrs = [
        "OlaFaceUnity.h",
        "OlaFURenderView.h",
        "OlaFURenderView+private.h"
    ],
    srcs = [
        "OlaFaceUnity.mm",
        "OlaFURenderView.mm",
    ],
    visibility = ["//visibility:public"],
    deps = [
         "//mediapipe/render/module/beauty:FaceMeshGPULibrary",
         "//mediapipe/render/core:core-ios",
         "@ios_opencv//:OpencvFramework",
    ],
    data = [
        "//mediapipe/graphs/face_mesh:face_mesh_mobile_gpu_prestream_ios.binarypb",
        "//mediapipe/modules/face_detection:face_detection_short_range.tflite",
        # "//mediapipe/modules/face_landmark:face_landmark.tflite",
        "//mediapipe/modules/face_landmark:face_landmark_with_attention.tflite",
        "//mediapipe/modules/selfie_segmentation:selfie_segmentation.tflite",
        "//mediapipe/render/module/beauty:whiten.png",
        "//mediapipe/render/module/beauty:skinGray.png",
        "//mediapipe/render/module/beauty:skinLookup.png",
    ],
     copts = select({
        "//mediapipe:apple": [
            "-x objective-c++",
            # "-fobjc-arc",  # enable reference-counting
        ],
        "//conditions:default": [],
    }),
    # alwayslink = True,
    sdk_frameworks = [
        "AVFoundation",
        "CoreGraphics",
        "CoreMedia",
        "UIKit",
        "OpenGLES",
        "MetalKit",
        "GLKit",
        "Metal",
        "CoreVideo",
        "IOSurface",
        "Foundation",
        "AssetsLibrary",
        "QuartzCore",
    ],
)


# ios_framework(
#     name = "OlaFaceUnityFramework",
#     hdrs = [
#         "OlaFaceUnity.h",
#         "OlaFURenderView.h",
#     ],
#     infoplists = ["Info.plist"],
#     bundle_id = "com.ola.olarender.develop",
#     families = ["iphone", "ipad"],
#     minimum_os_version = "11.0",
#     deps = [
#          ":OlaFaceUnityLibrary",
#          "@ios_opencv//:OpencvFramework",
#     ],
# )

# objc_library(
#     name = "OlaFaceUnityLibrary",
#     hdrs = [
#         "OlaFaceUnity.h",
#         "OlaFURenderView.h",
#         "OlaFURenderView+private.h"
#     ],
#     srcs = [
#         "OlaFaceUnity.mm",
#         "OlaFURenderView.mm",
#     ],
#     visibility = ["//visibility:public"],
#     deps = [
#          "//mediapipe/render/module/beauty:FaceMeshGPULibrary",
#          "//mediapipe/render/core:core-ios",
#          "@ios_opencv//:OpencvFramework",
#     ],
#     data = [
#         "//mediapipe/graphs/face_mesh:face_mesh_mobile_vFlip.binarypb",
#         "//mediapipe/modules/face_detection:face_detection_short_range.tflite",
#         "//mediapipe/modules/face_landmark:face_landmark_with_attention.tflite",
#         "//mediapipe/modules/selfie_segmentation:selfie_segmentation.tflite",
#         "//mediapipe/render/module/beauty:whiten.png",
#         "//mediapipe/render/module/beauty:skinGray.png",
#         "//mediapipe/render/module/beauty:skinLookup.png",
#     ],
#      copts = select({
#         "//mediapipe:apple": [
#             "-x objective-c++",
#             # "-fobjc-arc",  # enable reference-counting
#         ],
#         "//conditions:default": [],
#     }),
#     # alwayslink = True,
#     sdk_frameworks = [
#         "AVFoundation",
#         "CoreGraphics",
#         "CoreMedia",
#         "UIKit",
#         "OpenGLES",
#         "MetalKit",
#         "GLKit",
#         "Metal",
#         "CoreVideo",
#         "IOSurface",
#         "Foundation",
#         "AssetsLibrary",
#         "QuartzCore",
#     ],
# )


