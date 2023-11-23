#include "mediapipe/examples/desktop/fmesh.h"

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"

PGraph init_graph() { return new mediapipe::CalculatorGraph{}; }

void delete_graph(PGraph self) {
  delete reinterpret_cast<mediapipe::CalculatorGraph *>(self);
}

void delete_status_poller(StatusPoller self) {
  delete reinterpret_cast<absl::StatusOr<mediapipe::OutputStreamPoller> *>(
      self);
}

void prepare_graph(PGraph pgraph, const char *calculator_graph_config_file) {

  mediapipe::CalculatorGraph *graph =
      reinterpret_cast<mediapipe::CalculatorGraph *>(pgraph);

  std::string calculator_graph_config_contents;

  absl::Status get_content_status = mediapipe::file::GetContents(
      calculator_graph_config_file, &calculator_graph_config_contents);
  if (!get_content_status.ok()) {
    ABSL_LOG(ERROR) << "Failed to read graph config content: "
                    << get_content_status.message();
    exit(EXIT_FAILURE);
  }

  mediapipe::CalculatorGraphConfig config =
      mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(
          calculator_graph_config_contents);

  ABSL_LOG(INFO) << "Initialize the calculator graph.";

  absl::Status ini_graph_status = graph->Initialize(config);
  if (!ini_graph_status.ok()) {
    ABSL_LOG(ERROR) << "Failed to initialize graph: "
                    << ini_graph_status.message();
    exit(EXIT_FAILURE);
  }
}

StatusPoller prepare_poller(PGraph pgraph, const char *kOutputStream) {
  mediapipe::CalculatorGraph *graph =
      reinterpret_cast<mediapipe::CalculatorGraph *>(pgraph);
  absl::StatusOr<mediapipe::OutputStreamPoller> *status_poller =
      new absl::StatusOr<mediapipe::OutputStreamPoller>(
          graph->AddOutputStreamPoller(kOutputStream));
  if (status_poller->ok()) {
    return status_poller;
  } else {
    ABSL_LOG(ERROR) << "Failed to prepare poller: " << status_poller->status();
    exit(EXIT_FAILURE);
  }
}

void start_run(PGraph pgraph) {
  mediapipe::CalculatorGraph *graph =
      reinterpret_cast<mediapipe::CalculatorGraph *>(pgraph);
  absl::Status status = graph->StartRun({});
  if (!status.ok()) {
    ABSL_LOG(ERROR) << "Failed to graph start run: " << status.message();
    exit(EXIT_FAILURE);
  }
}

void shut_down(PGraph pgraph, const char *kInputStream) {
  ABSL_LOG(INFO) << "Shutting down.";
  mediapipe::CalculatorGraph *graph =
      reinterpret_cast<mediapipe::CalculatorGraph *>(pgraph);

  absl::Status close_in_stream_status = graph->CloseInputStream(kInputStream);
  if (!close_in_stream_status.ok()) {
    ABSL_LOG(INFO) << "Failed close input stream :(";
    ABSL_LOG(INFO) << "Failed shut down :(";
    exit(EXIT_FAILURE);
  }
  absl::Status wait_status = graph->WaitUntilDone();
  if (!wait_status.ok()) {
    ABSL_LOG(INFO) << "Failed shut down :(";
    exit(EXIT_FAILURE);
  }
}

int process_frame(int rows, int cols, int channels, const int type,
                  unsigned char *data, unsigned char *buffer,
                  StatusPoller spoller, PGraph pgraph,
                  const char *kInputStream) {

  mediapipe::CalculatorGraph *graph =
      reinterpret_cast<mediapipe::CalculatorGraph *>(pgraph);
  absl::StatusOr<mediapipe::OutputStreamPoller> *status_poller =
      reinterpret_cast<absl::StatusOr<mediapipe::OutputStreamPoller> *>(
          spoller);
  // Construct camera frame from raw data
  // Mat(nrows, ncols, type[, fillValue])
  cv::Mat camera_frame(rows, cols, type, data);
  // Wrap Mat into an ImageFrame.
  auto input_frame = absl::make_unique<mediapipe::ImageFrame>(
      mediapipe::ImageFormat::SRGB, camera_frame.cols, camera_frame.rows,
      mediapipe::ImageFrame::kDefaultAlignmentBoundary);
  cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());
  camera_frame.copyTo(input_frame_mat);

  // Send image packet into the graph.
  size_t frame_timestamp_us =
      (double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e6;
  absl::Status send_status = graph->AddPacketToInputStream(
      kInputStream, mediapipe::Adopt(input_frame.release())
                        .At(mediapipe::Timestamp(frame_timestamp_us)));
  if (!send_status.ok()) {
    ABSL_LOG(ERROR) << "Failed to send image packet into the grph: "
                    << send_status.message();
    exit(EXIT_FAILURE);
  }

  // Get the graph result packet, or stop if that fails.
  mediapipe::Packet packet;
  if (!(**status_poller).Next(&packet)) {
    return -1;
  }
  auto &output_frame = packet.Get<mediapipe::ImageFrame>();

  // Convert back to opencv for display or saving.
  cv::Mat output_frame_mat = mediapipe::formats::MatView(&output_frame);
  cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGB2BGR);
  memcpy(buffer, output_frame_mat.data, rows * cols * channels);
  return 0;
}
