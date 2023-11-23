#ifndef MEDIAPIPE_FMESH_H
#define MEDIAPIPE_FMESH_H

typedef void *PGraph;
typedef void *StatusPoller;

extern "C" PGraph init_graph();

extern "C" void delete_graph(PGraph self);

extern "C" void delete_status_poller(StatusPoller self);

extern "C" void prepare_graph(PGraph pgraph,
                              const char *calculator_graph_config_file);

extern "C" StatusPoller prepare_poller(PGraph pgraph,
                                       const char *kOutputStream);

extern "C" void start_run(PGraph pgraph);

extern "C" void shut_down(PGraph pgraph, const char *kInputStream);

extern "C" int process_frame(int rows, int cols, int channels, const int type,
                             unsigned char *data, unsigned char *buffer,
                             StatusPoller spoller, PGraph pgraph,
                             const char *kInputStream);

#endif // MEDIAPIPE_FMESH_H
