package(default_visibility = ["//visibility:public"])


BEAUTYFILTERS_SRCS = [
        "BeautyAdjustFilter.cpp",
        "BeautyComplexion.cpp",
        "BeautyGaussBlurFilter.cpp",
        "BeautyGaussPassFilter.cpp",
        "BeautyHighPassFilter.cpp",
        "OlaBeautyFilterV2.cpp",
        "FaceDistortionFilter.cpp",
        "BeautySharpenFilter.cpp"
]

BEAUTYFILTERS_HDRS = [
        "BeautyAdjustFilter.hpp",
        "BeautyComplexion.hpp",
        "BeautyGaussBlurFilter.hpp",
        "BeautyGaussPassFilter.hpp",
        "BeautyHighPassFilter.hpp",
        "BeautyV2.hpp",
        "OlaBeautyFilterV2.hpp",
        "FaceDistortionFilter.hpp",
        "BeautySharpenFilter.hpp"
]

cc_library(
    name = "BeautyFilters",
    srcs = BEAUTYFILTERS_SRCS,
    hdrs = BEAUTYFILTERS_HDRS,
    visibility = ["//visibility:public"],
    alwayslink = True,
    linkstatic = True,
    deps = [
    ] + select({
        "//mediapipe:apple": [
            "//mediapipe/render/core:core-ios",
         ],
        "//conditions:default": [
            "//mediapipe/render/core:core",
        ],
    }),
    copts = select({
        "//mediapipe:apple": [
            "-x objective-c++",
            # "-fobjc-arc",  # enable reference-counting
        ],
        "//conditions:default": ["-std=c++17"],
    }),
)