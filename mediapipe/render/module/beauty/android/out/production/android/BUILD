load("//mediapipe/render/module/beauty/android/java/com/ola/frameworks:opipe_aar.bzl", "opipe_aar")

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "libopipe_jni.so",
    linkshared = 1,
    linkstatic = 1,
    deps = [
        "//mediapipe/render/module/beauty/android/java/com/ola/frameworks/jni:opipe_framework_jni",
    ],
)

cc_library(
    name = "opipe_jni_lib",
    srcs = [":libopipe_jni.so"],
    visibility = ["//visibility:public"],
    alwayslink = 1,
)

opipe_aar(
    name = "opipe_jni",
    srcs = glob(["*.java"]),
    gen_lib = False,
    assets = [
        "//mediapipe/render/android/camera/java/com/ola/olamera:camera_core",
        "//mediapipe/graphs/face_mesh:face_mesh_mobile_gpu.binarypb",
        "//mediapipe/modules/face_detection:face_detection_short_range.tflite",
        "//mediapipe/modules/face_landmark:face_landmark_with_attention.tflite",
        "//mediapipe/modules/selfie_segmentation:selfie_segmentation.tflite",
        "//mediapipe/render/module/beauty:whiten.png",
    ],
    assets_dir = "",
    proguard_specs = ["proguard.pgcfg"],
    # deps = [
    #     "//mediapipe/render/module/render_queue:olarender",
    # ],
)
