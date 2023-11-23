#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

#include "mediapipe/examples/desktop/fmesh.h"

ABSL_FLAG(std::string, calculator_graph_config_file, "",
          "Name of file containing text format CalculatorGraphConfig proto.");
ABSL_FLAG(std::string, input_video_path, "",
          "Full path of video to load. "
          "If not provided, attempt to use a webcam.");
ABSL_FLAG(std::string, output_video_path, "",
          "Full path of where to save result (.mp4 only). "
          "If not provided, show result in a window.");

void prepare_capture(cv::VideoCapture &capture, const bool &load_video,
                     const bool &save_video, const char *input_video_path,
                     const char *kWindowName) {
  ABSL_LOG(INFO) << "Initialize the camera or load the video.";

  if (load_video) {
    capture.open(input_video_path);
  } else {
    capture.open(0);
  }
  if (!capture.isOpened()) {
    ABSL_LOG(ERROR) << "Failed to open video capture";
    exit(EXIT_FAILURE);
  }
  if (!save_video) {
    cv::namedWindow(kWindowName, /*flags=WINDOW_AUTOSIZE*/ 1);
#if (CV_MAJOR_VERSION >= 3) && (CV_MINOR_VERSION >= 2)
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    capture.set(cv::CAP_PROP_FPS, 30);
#endif
  }
}

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  absl::ParseCommandLine(argc, argv);

  constexpr char kInputStream[] = "input_video";
  constexpr char kOutputStream[] = "output_video";
  constexpr char kWindowName[] = "MediaPipe";

  const bool save_video = !absl::GetFlag(FLAGS_output_video_path).empty();
  const bool load_video = !absl::GetFlag(FLAGS_input_video_path).empty();

  char calculator_graph_config_file[128];
  strcpy(calculator_graph_config_file,
         absl::GetFlag(FLAGS_calculator_graph_config_file).data());

  char input_video_path[128];
  strcpy(input_video_path, absl::GetFlag(FLAGS_input_video_path).data());

  char output_video_path[128];
  strcpy(output_video_path, absl::GetFlag(FLAGS_output_video_path).data());

  int if_break = 0;
  bool grab_frames = true;
  cv::VideoCapture capture;
  cv::VideoWriter writer;

  PGraph graph = init_graph();
  prepare_graph(graph, calculator_graph_config_file);
  prepare_capture(capture, load_video, save_video, input_video_path,
                  kWindowName);
  StatusPoller status_poller = prepare_poller(graph, kOutputStream);
  start_run(graph);

  cv::Mat camera_frame_raw;
  cv::Mat camera_frame;

  while (grab_frames) {
    // Capture opencv camera or video frame.
    capture >> camera_frame_raw;
    if (camera_frame_raw.empty()) {
      if (!load_video) {
        ABSL_LOG(INFO) << "Ignore empty frames from camera.";
        continue;
      }
      ABSL_LOG(INFO) << "Empty frame, end of video reached.";
      break;
    }
    cv::cvtColor(camera_frame_raw, camera_frame, cv::COLOR_BGR2RGB);
    if (!load_video) {
      cv::flip(camera_frame, camera_frame, /*flipcode=HORIZONTAL*/ 1);
    }

    // ACTUAL PROCESSING
    // Mat(nrows, ncols, type[, fillValue])
    // 480 640 3 16 (CV_8U)
    int rows = camera_frame.rows;
    int cols = camera_frame.cols;
    int channels = camera_frame.channels();
    const int type = camera_frame.type();
    unsigned char *data = camera_frame.data;
    // At the moment assume the video resolution and data type are not changed
    // So it is enough to return only pointer to data and then construct cv::Mat
    cv::Mat output_frame_mat = camera_frame.clone();
    if_break =
        process_frame(rows, cols, channels, type, data, output_frame_mat.data,
                      status_poller, graph, kInputStream);

    if (if_break < 0)
      break;
    // END ACTUAL PROCESSING

    if (save_video) {
      if (!writer.isOpened()) {
        ABSL_LOG(INFO) << "Prepare video writer.";
        writer.open(output_video_path,
                    mediapipe::fourcc('a', 'v', 'c', '1'), // .mp4
                    capture.get(cv::CAP_PROP_FPS), output_frame_mat.size());
        if (!writer.isOpened()) {
          ABSL_LOG(ERROR) << "Failed to wrine to file, file closed.";
          exit(EXIT_FAILURE);
        }
      }
      writer.write(output_frame_mat);
    } else {
      cv::imshow(kWindowName, output_frame_mat);
      // Press any key to exit.
      const int pressed_key = cv::waitKey(5);
      if (pressed_key >= 0 && pressed_key != 255)
        grab_frames = false;
    }
  }

  if (writer.isOpened())
    writer.release();
  shut_down(graph, kInputStream);
  delete_graph(graph);
  delete_status_poller(status_poller);
  ABSL_LOG(INFO) << "Success!";
  return EXIT_SUCCESS;
}
