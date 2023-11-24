use clap::Parser;
use fmeshrs;
use opencv::core::flip;
use opencv::highgui::{destroy_all_windows, imshow, named_window, wait_key, WINDOW_AUTOSIZE};
use opencv::hub_prelude::MatTraitConst;
use opencv::imgproc::{cvt_color, COLOR_BGR2RGB};
use opencv::prelude::{
    Mat, VideoCaptureTrait, VideoCaptureTraitConst, VideoWriterTrait, VideoWriterTraitConst,
};
use opencv::videoio::{
    VideoCapture, VideoWriter, CAP_ANY, CAP_PROP_FPS, CAP_PROP_FRAME_HEIGHT, CAP_PROP_FRAME_WIDTH,
};
use std::ffi::CString;

#[derive(Parser, Debug)]
#[command(author, version, about, long_about=None)]
struct Args {
    /// Name of file containing text format CalculatorGraphConfig proto.
    #[arg(short, long)]
    calculator_graph_config_file: String,

    /// Full path of video to load. If not provided, attempt to use a webcam.
    #[arg(short, long)]
    input_video_path: Option<String>,

    /// Full path of where to save result (.mp4 only). If not provided, show result in a window.
    #[arg(short, long)]
    output_video_path: Option<String>,
}

#[allow(non_snake_case)]
fn prepare_capture(
    capture: &mut VideoCapture,
    load_video: bool,
    save_video: bool,
    input_videp_path: &str,
    kWindowName: &str,
) {
    println!("Initialize the camera of load the video.");
    if load_video {
        _ = capture.open_file_def(input_videp_path);
    } else {
        _ = capture.open(0, CAP_ANY);
    }

    match capture.is_opened() {
        Ok(_) => (),
        Err(_) => {
            println!("Failed to open video capture");
            std::process::exit(1);
        }
    }

    if !save_video {
        _ = named_window(kWindowName, WINDOW_AUTOSIZE);
        _ = capture.set(CAP_PROP_FRAME_WIDTH, 640 as f64);
        _ = capture.set(CAP_PROP_FRAME_HEIGHT, 480 as f64);
        _ = capture.set(CAP_PROP_FPS, 30 as f64);
    }
}

#[allow(non_snake_case)]
fn main() {
    let args = Args::parse();

    let (input_video_path, load_video) = match args.input_video_path {
        Some(path) => (path, true),
        None => ("".into(), false),
    };
    let (output_video_path, save_video) = match args.output_video_path {
        Some(path) => (path, true),
        None => ("".into(), false),
    };

    let kInputStream = "input_video";
    let kOutputStream = "output_video";
    let kWindowName = "MediaPipe";
    let fourcc = VideoWriter::fourcc('m', 'p', '4', 'v').unwrap();
    let mut if_break: libc::c_int;

    let mut capture = VideoCapture::default().unwrap();
    let mut writer = VideoWriter::default().unwrap();

    // init graph
    let pgraph = unsafe { fmeshrs::init_graph() };
    // prepare graph
    let calc_path = CString::new(&args.calculator_graph_config_file[..])
        .expect("CString failed on parsing calculator_graph_config_file string")
        .into_raw();
    unsafe { fmeshrs::prepare_graph(pgraph, calc_path) };

    // prepare capture
    prepare_capture(
        &mut capture,
        load_video,
        save_video,
        &input_video_path,
        kWindowName,
    );

    // prepare poller
    let output_stream = CString::new(kOutputStream)
        .expect("CString failed to parce kOutputStream string")
        .into_raw();
    let input_stream = CString::new(kInputStream)
        .expect("CString failed to parce kInputStream string")
        .into_raw();
    let status_poller = unsafe { fmeshrs::prepare_poller(pgraph, output_stream) };

    // start run
    unsafe { fmeshrs::start_run(pgraph) };

    let mut camera_frame_raw = Mat::default();
    let mut frame_flipped = Mat::default();
    let mut camera_frame = Mat::default();
    let mut output_frame: Mat;

    let mut pressed_key: Result<i32, _>;
    let quit = 113; // key 'q'
    loop {
        match capture.read(&mut camera_frame_raw) {
            Ok(res) if !res => {
                if !load_video {
                    println!("Ignore empty frames from camera.");
                    continue;
                }
                println!("Empty frame, end of video reached.");
                break;
            }
            _ => {}
        };
        _ = cvt_color(&camera_frame_raw, &mut camera_frame, COLOR_BGR2RGB, 0);
        if !load_video {
            _ = flip(&camera_frame, &mut frame_flipped, 1);
            camera_frame = frame_flipped.clone();
        }

        // actual processing
        let rows = camera_frame.rows();
        let cols = camera_frame.cols();
        let channels = camera_frame.channels();
        let typ = camera_frame.typ();
        // 480, 640, 3, 16
        // println!("{rows}, {cols}, {channels}, {typ}");
        output_frame = camera_frame.clone();
        let buffer = output_frame.data() as *mut u8;
        let data = camera_frame.data() as *const u8;
        if_break = unsafe {
            fmeshrs::process_frame(
                rows,
                cols,
                channels,
                typ,
                data,
                buffer,
                status_poller,
                pgraph,
                input_stream,
            )
        };
        if if_break < 0 {
            break;
        }
        if save_video {
            match writer.is_opened() {
                Ok(res) if !res => {
                    println!("Prepare video writer.");
                    writer
                        .open(
                            &output_video_path,
                            fourcc,
                            capture.get(CAP_PROP_FPS).unwrap(),
                            output_frame.size().unwrap(),
                            true,
                        )
                        .unwrap_or_else(|_| {
                            println!("Failed to write to file. File closed.");
                            std::process::exit(1);
                        });
                }
                _ => {}
            }
            _ = writer.write(&output_frame);
        } else {
            _ = imshow(kWindowName, &output_frame);
        }
        pressed_key = wait_key(1);
        match pressed_key {
            Ok(q) if q == quit => {
                println!("quitting...");
                break;
            }
            _ => (),
        }
    }

    _ = capture.release();
    _ = writer.release();
    _ = destroy_all_windows();

    // shut down
    unsafe {
        fmeshrs::shut_down(pgraph, input_stream);
    }

    // delete graph
    unsafe {
        fmeshrs::delete_graph(pgraph);
    };

    // delete status poller
    unsafe { fmeshrs::delete_status_poller(status_poller) };
    println!("SUCCESS!");
}
