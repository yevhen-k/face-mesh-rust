#!/usr/bin/env bash
set -xue

# copy c++ code, headers, etc. to the mediapipe submodule
# currently only Linux is configured
# see
# - https://developers.google.com/mediapipe/framework/getting_started/cpp
# - https://github.com/google/mediapipe/tree/master/third_party
# for more details
cp -r thirdparty/* mediapipe-git/

cd mediapipe-git

# build demo cpp app without GPU support
bazel build --subcommands --copt="-fPIC" --sandbox_debug --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/demo:demo

# run simple test if any
if [ -z ${TEST_DEMO+x} ]; then
	echo "Skipping tests"
else
	if [[ $TEST_DEMO -eq 1 ]]; then
		echo "Testing WebCam"
		GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/demo/demo --calculator_graph_config_file=mediapipe/graphs/face_mesh/face_mesh_desktop_live.pbtxt
	fi
	if [[ $TEST_DEMO -eq 2 ]]; then
		echo "Testing with local video file and OpenCV window"
		GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/demo/demo --calculator_graph_config_file=mediapipe/graphs/face_mesh/face_mesh_desktop_live.pbtxt --input_video_path=../test.mp4
	fi
	if [[ $TEST_DEMO -eq 3 ]]; then
		echo "Testing with local video file and save to local video file"
		GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/demo/demo --calculator_graph_config_file=mediapipe/graphs/face_mesh/face_mesh_desktop_live.pbtxt --input_video_path=../test.mp4 --output_video_path=../out.mp4
	fi
fi

# prepare `.params` file to link library
# (this is a hackish way, probably there is a proper
# way to do this in bazel, but I haven't found it)
rm -f link.params libfmesh.so
touch link.params
cat <<EOT >>link.params
-shared
-fPIC
-o
libfmesh.so
EOT

tail -n +3 bazel-out/k8-fastbuild/bin/mediapipe/examples/desktop/demo/demo-2.params >>link.params

gcc @link.params

# move linked shared library to dest
mv libfmesh.so ../lib/

# copy rest of the necessary files (tflite models)
cp -H bazel-bin/mediapipe/modules/face_detection/face_detection_short_range.tflite ../mediapipe/modules/face_detection/face_detection_short_range.tflite
cp -H bazel-bin/mediapipe/modules/face_landmark/face_landmark_with_attention.tflite ../mediapipe/modules/face_landmark/face_landmark_with_attention.tflite

echo "SUCCESS!"
