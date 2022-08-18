# Copyright 2019 The MediaPipe Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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
        "face_mesh_jni.cc",
    ] + select({
        "//conditions:default": [],
        "//mediapipe:android": [
            
        ],
    }) + select({
        "//conditions:default": [
        ],
        "//mediapipe/gpu:disable_gpu": [],
    }),
    hdrs = [
        "face_mesh_jni.h",
    ] + select({
        "//conditions:default": [],
        "//mediapipe:android": [
        ],
    }) + select({
        "//conditions:default": [
        ],
        "//mediapipe/gpu:disable_gpu": [],
    }),
    linkopts = select({
        "//conditions:default": [],
        "//mediapipe:android": [
            "-ljnigraphics",
            "-lEGL",  # This is needed by compat_jni even if GPU is disabled.
        ],
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//mediapipe/render/core/jni:opipe_android_framework_jni",
        "//mediapipe/render/module/beauty:FaceMeshGPULibrary",
        "//mediapipe/framework/port:logging",
    ],
    alwayslink = 1,
)
