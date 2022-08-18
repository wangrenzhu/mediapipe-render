licenses(["notice"])

package(
    default_visibility = ["//visibility:public"],
    features = ["no_layering_check"],
)

alias(
    name = "opipe_android_framework_jni",
    actual = ":opipe_framework_jni",
)

cc_library(
    name = "opipe_framework_jni",
    srcs = [
        "JavaHolder.cpp",
        "JNIEnvAttach.cpp",
        "JNIUtils.cpp",
    ],
    hdrs = [
        "JavaHolder.h",
        "JavaObjectMap.hpp",
        "JNIEnvAttach.h",
        "JNIUtils.h",
    ],
    linkopts = select({
        "//conditions:default": [],
        "//mediapipe:android": [
            "-ljnigraphics",
            "-lEGL",  # This is needed by compat_jni even if GPU is disabled.
        ],
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/framework/port:logging",
    ],
    alwayslink = 1,
)
