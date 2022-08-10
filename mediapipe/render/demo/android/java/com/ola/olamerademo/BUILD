# Copyright 2020 The MediaPipe Authors.
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

package(default_visibility = ["//visibility:private"])

cc_binary(
    name = "libola_render_jni.so",
    srcs = [
        "//mediapipe/render/module/render_queue:olarender",
    ],
    linkshared = 1,
    linkstatic = 1,
)

cc_library(
    name = "ola_render_jni_lib",
    srcs = [":libola_render_jni.so"],
    alwayslink = 1,
)

aar_import(
    name = "opipe_jni.aar",
    aar = "//mediapipe/render/module/beauty/android/java/com/ola/frameworks:opipe_jni",
)

android_binary(
    name = "olamerademo",
    srcs = glob(["*.java"]),
    assets = [
        ":rbga_2_yuv_frag.frag",
        ":rgba_2_yuv_vertex.vert",
    ],
    assets_dir = "",
    manifest = ":AndroidManifest.xml",
    multidex = "native",
    resource_files = glob(["res/**"]),
    visibility = ["//visibility:public"],
    deps = [
        # ":ola_render_jni_lib",
        "//mediapipe/render/android/camera/java/com/ola/olamera:camera_core",
        "opipe_jni.aar",
        "//third_party:androidx_annotation",
        "//third_party:lifecycle_livedata",
        "//third_party:androidx_appcompat",
        "//third_party:lifecycle_livedata_core",
        "//third_party:androidx_core",
        "//third_party:androidx_lifecycle_common",
        "//third_party:androidx_lifecycle_runtime",
        "@maven//:com_google_guava_guava",
    ],
)
