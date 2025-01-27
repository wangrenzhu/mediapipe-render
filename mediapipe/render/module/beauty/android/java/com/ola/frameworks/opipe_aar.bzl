#Package: com.ola.render
#Description:
#Author: 王强

load("@build_bazel_rules_android//android:rules.bzl", "android_binary", "android_library")

def opipe_aar(
        name,
        srcs = [],
        assets = [],
        gen_lib = True,
        proguard_specs = [],
        assets_dir = ""):
    _opipe_jni(
        gen_lib = gen_lib,
        name = name + "_jni",
    )

    native.genrule(
        name = name + "_aar_manifest_generator",
        outs = ["AndroidManifest.xml"],
        cmd = """
cat > $(OUTS) <<EOF
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="com.ola.render">
    <uses-sdk
        android:minSdkVersion="16"
        android:targetSdkVersion="27" />
</manifest>
EOF
""",
    )

    android_library(
        name = name + "_android_lib",
        srcs = srcs,
        manifest = "AndroidManifest.xml",
        proguard_specs = proguard_specs,
        assets = assets,
        assets_dir = assets_dir,
        deps = [
            ":" + name + "_jni_cc_lib",
            "//third_party:androidx_annotation",
            "//mediapipe/render/android/camera/java/com/ola/olamera:camera_core",
        ]+ select({
            "//conditions:default": [":" + name + "_jni_opencv_cc_lib",
                    "@maven//:androidx_concurrent_concurrent_futures",
                    "@maven//:com_google_guava_guava",],
        }),
    )

    _ola_aar_with_jni(name, name + "_android_lib")

def _opipe_jni(gen_lib, name):
    if gen_lib:
        native.cc_binary(
            name = "libopipe_jni.so",
            linkshared = 1,
            linkstatic = 1,
            deps = [
                "//mediapipe/render/module/beauty/android/java/com/ola/framework/jni:opipe_framework_jni",
            ],
        )

    native.cc_library(
        name = name + "_cc_lib",
        srcs = [":libopipe_jni.so"],
        alwayslink = 1,
    )

    native.cc_library(
        name = name + "_opencv_cc_lib",
        srcs = select({
            "//mediapipe:android_arm64": ["@android_opencv//:libopencv_java3_so_arm64-v8a"],
            "//mediapipe:android_armeabi": ["@android_opencv//:libopencv_java3_so_armeabi-v7a"],
            "//mediapipe:android_arm": ["@android_opencv//:libopencv_java3_so_armeabi-v7a"],
            "//mediapipe:android_x86": ["@android_opencv//:libopencv_java3_so_x86"],
            "//mediapipe:android_x86_64": ["@android_opencv//:libopencv_java3_so_x86_64"],
            "//conditions:default": [],
        }),
        alwayslink = 1,
    )

def _ola_aar_with_jni(name, android_library):
    # Generates dummy AndroidManifest.xml for dummy apk usage
    # (dummy apk is generated by <name>_dummy_app target below)
    native.genrule(
        name = name + "_binary_manifest_generator",
        outs = [name + "_generated_AndroidManifest.xml"],
        cmd = """
cat > $(OUTS) <<EOF
<manifest
  xmlns:android="http://schemas.android.com/apk/res/android"
  package="dummy.package.for.so">
  <uses-sdk android:minSdkVersion="21"/>
</manifest>
EOF
""",
    )

    # Generates dummy apk including .so files.
    # We extract out .so files and throw away the apk.
    android_binary(
        name = name + "_dummy_app",
        manifest = name + "_generated_AndroidManifest.xml",
        custom_package = "dummy.package.for.so",
        multidex = "native",
        deps = [android_library],
    )

    native.genrule(
        name = name,
        srcs = [android_library + ".aar", name + "_dummy_app_unsigned.apk"],
        outs = [name + ".aar"],
        tags = ["manual"],
        cmd = """
cp $(location {}.aar) $(location :{}.aar)
chmod +w $(location :{}.aar)
origdir=$$PWD
cd $$(mktemp -d)
unzip $$origdir/$(location :{}_dummy_app_unsigned.apk) "lib/*"
cp -r lib jni
zip -r $$origdir/$(location :{}.aar) jni/*/*.so
""".format(android_library, name, name, name, name),
    )
