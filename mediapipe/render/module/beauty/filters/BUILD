package(default_visibility = ["//visibility:public"])


BEAUTYFILTERS_SRCS = [
        "BilateralAdjustFilter.cpp",
        "OlaBeautyFilter.cpp",
        "UnSharpMaskFilter.cpp",
]

BEAUTYFILTERS_HDRS = [
        "BilateralAdjustFilter.hpp",
        "OlaBeautyFilter.hpp",
        "UnSharpMaskFilter.hpp",
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
            "//mediapipe/render/module/beauty/filters/BeautyV2:BeautyFilters",
         ],
        "//conditions:default": [
            "//mediapipe/render/core:core",
            "//mediapipe/render/module/beauty/filters/BeautyV2:BeautyFilters",
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