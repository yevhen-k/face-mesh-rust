use libc;

pub enum PGraph {}
pub enum StatusPoller {}

extern "C" {

    pub fn init_graph() -> *mut PGraph;

    pub fn delete_graph(pgraph: *mut PGraph) -> libc::c_void;

    pub fn delete_status_poller(spoller: *mut StatusPoller) -> libc::c_void;

    pub fn prepare_graph(
        pgraph: *mut PGraph,
        calculator_graph_config_file: *const libc::c_char,
    ) -> libc::c_void;

    pub fn prepare_poller(
        pgraph: *mut PGraph,
        kOutputStream: *const libc::c_char,
    ) -> *mut StatusPoller;

    pub fn start_run(pgraph: *mut PGraph) -> libc::c_void;

    pub fn shut_down(pgraph: *mut PGraph, kInputStream: *const libc::c_char) -> libc::c_void;

    pub fn process_frame(
        rows: libc::c_int,
        cols: libc::c_int,
        channels: libc::c_int,
        r#type: libc::c_int,
        data: *const u8,
        buffer: *mut u8,
        spoller: *mut StatusPoller,
        pgraph: *mut PGraph,
        kInputStream: *const libc::c_char,
    ) -> libc::c_int;

}

// Simple tests just to check linking and basic functionality
#[cfg(test)]
mod tests {
    use super::*;
    use std::ffi::CString;

    #[test]
    fn init_graph_test() {
        let pgraph: *mut PGraph = unsafe { init_graph() };
        unsafe { delete_graph(pgraph) };
    }

    #[test]
    fn prepare_graph_test() {
        unsafe {
            let pgraph = init_graph();
            let calculator_graph_config_file = "./face_mesh_desktop_live.pbtxt";
            let c_conf_path = CString::new(calculator_graph_config_file)
                .expect("CString:: new failed")
                .into_raw();
            prepare_graph(pgraph, c_conf_path);
            delete_graph(pgraph);
        }
    }
}
