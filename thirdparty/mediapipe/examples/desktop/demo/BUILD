package(default_visibility = ["//mediapipe/examples:__subpackages__"])

cc_binary(
    name = "demo",
    srcs = ["demo.cc"],
    data = [
        "//mediapipe/modules/face_detection:face_detection_short_range.tflite",
        "//mediapipe/modules/face_landmark:face_landmark_with_attention.tflite",
    ],
    # linkshared = False,
    # linkstatic = False,
    deps = [
        "//mediapipe/examples/desktop:fmesh",
        "//mediapipe/graphs/face_mesh:desktop_live_calculators",
    ],
)

# Linux only
# cc_binary(
#     name = "demo_gpu",
#     data = [
#         "//mediapipe/modules/face_detection:face_detection_short_range.tflite",
#         "//mediapipe/modules/face_landmark:face_landmark_with_attention.tflite",
#     ],
#     deps = [
#         "//mediapipe/examples/desktop:demo_run_graph_main_gpu",
#         "//mediapipe/graphs/face_mesh:desktop_live_gpu_calculators",
#     ],
# )
