load("//mediapipe/framework/tool:mediapipe_graph.bzl", "mediapipe_binary_graph")
load("@build_bazel_rules_apple//apple:ios.bzl", "ios_framework")
package(default_visibility = ["//visibility:private"])

cc_library(
    name = "olamodule_common_library",
    srcs = ["ola_graph.cc"],
    hdrs = ["ola_graph.h"],
    visibility = ["//mediapipe/framework:mediapipe_internal"],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework:mediapipe_profiling",
        "//mediapipe/framework/formats:image",
        "//mediapipe/framework/port:map_util",
        "//mediapipe/framework/port:ret_check",
        "//mediapipe/framework/port:source_location",
        "//mediapipe/framework/port:status",
        "//mediapipe/framework/port:statusor",
        "//mediapipe/framework/port:threadpool",
        "//mediapipe/framework/port:logging",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/gpu:gl_base",
        "//mediapipe/gpu:gpu_buffer",
        "//mediapipe/gpu:gl_texture_buffer",
        "//mediapipe/gpu:gpu_shared_data_internal",
        "//mediapipe/gpu:graph_support",
        "//mediapipe/util:cpu_util",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@google_toolbox_for_mac//:GTM_Defines",
    ] + select({
        "//mediapipe:ios": [
            "//mediapipe/gpu:pixel_buffer_pool_util",
            "//mediapipe/objc:CFHolder",
            "//mediapipe/objc:Weakify",
            "//mediapipe/objc:util",
        ],
        "//conditions:default": [
        ],
    }),
    copts = select({
        "//mediapipe:apple": [
            "-x objective-c++",
            "-Wno-shorten-64-to-32",
            "-fobjc-arc",  # enable reference-counting
        ],
        "//conditions:default": ["-std=c++17"],
    }),
    alwayslink = True,
)
