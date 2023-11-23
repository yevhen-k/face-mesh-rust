package(
    default_visibility = [
        "//visibility:public",
    ],
)

cc_library(
    name = "fmesh",
    srcs = ["fmesh.cpp"],
    hdrs = ["fmesh.h"],
    copts = [
        "-shared",
        "-fPIC",
    ],
    deps = [
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/port:file_helpers",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/port:opencv_video",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "//mediapipe/util:resource_util",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log:absl_log",
    ],
)

# Linux only.
# Must have a GPU with EGL support:
# ex: sudo apt-get install mesa-common-dev libegl1-mesa-dev libgles2-mesa-dev
# (or similar nvidia/amd equivalent)
# cc_library(
#     name = "demo_run_graph_main_gpu",
#     srcs = ["demo_run_graph_main_gpu.cc"],
#     deps = [
#         "//mediapipe/framework:calculator_framework",
#         "//mediapipe/framework/formats:image_frame",
#         "//mediapipe/framework/formats:image_frame_opencv",
#         "//mediapipe/framework/port:file_helpers",
#         "//mediapipe/framework/port:opencv_highgui",
#         "//mediapipe/framework/port:opencv_imgproc",
#         "//mediapipe/framework/port:opencv_video",
#         "//mediapipe/framework/port:parse_text_proto",
#         "//mediapipe/framework/port:status",
#         "//mediapipe/gpu:gl_calculator_helper",
#         "//mediapipe/gpu:gpu_buffer",
#         "//mediapipe/gpu:gpu_shared_data_internal",
#         "//mediapipe/util:resource_util",
#         "@com_google_absl//absl/flags:flag",
#         "@com_google_absl//absl/flags:parse",
#         "@com_google_absl//absl/log:absl_log",
#     ],
# )
