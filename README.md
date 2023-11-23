# Demo of Mediapipe FaceMesh with Rust

The repo demonstrates proof-of-concept of making shared library in Mediapipe (see [TODO](#TODO) section) and running Mediapipe apps with Rust.

## Quick Start

### Requirements
- bazel
- opencv

### Initialize Repo

Clone the repo:
```bash
git clone --recurse-submodules https://github.com/yevhen-k/face-mesh-rust.git
```

Initialize prerequisites:
```bash
./init.sh
```

The following vars are optional to make simple tests of cpp demo: `TEST_DEMO=1`, `TEST_DEMO=2`, `TEST_DEMO=3`:
```bash
TEST_DEMO=1 ./init.sh
```

See [init.sh](init.sh) for more details.

### Start Rust Demo

To see help:
```bash
LD_LIBRARY_PATH=./lib/ cargo run --bin demo -- --help
```

To test FaceMesh with your WebCam:
```bash
LD_LIBRARY_PATH=./lib/ cargo run --bin demo -- --calculator-graph-config-file=thirdparty/mediapipe/graphs/face_mesh/face_mesh_desktop_live.pbtxt
```

To test FaceMesh on a video file with output to opencv window:
```bash
LD_LIBRARY_PATH=./lib/ cargo run --bin demo -- --calculator-graph-config-file=thirdparty/mediapipe/graphs/face_mesh/face_mesh_desktop_live.pbtxt --input-video-path=test.mp4
```

To test FaceMesh on video and save result as a video:
```bash
LD_LIBRARY_PATH=./lib/ cargo run --bin demo -- --calculator-graph-config-file=thirdparty/mediapipe/graphs/face_mesh/face_mesh_desktop_live.pbtxt --input-video-path=test.mp4 --output-video-path=out.mp4
```

### Run Test
```bash
LD_LIBRARY_PATH=./lib cargo test
```

## TODO
1. Make static link with `fmesh` library.
2. Make GPU-accelerated app.
