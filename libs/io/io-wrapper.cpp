#include <falcon-typing/FFIHelpers.hpp>
#include <atomic>
#include <cstdio>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <fcntl.h>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

// ── StdoutCapture ─────────────────────────────────────────────────────────────
// Redirects stdout to an internal pipe so that lines written by any code
// (Falcon or C++) can be intercepted, buffered, and then forwarded to the
// original stdout.  The captured text remains accessible via Drain().
//
// Thread-safety: a single mutex guards the internal buffer and the dup2 state.
// Only one capture may be active at a time per process.

struct StdoutCapture {
    int      pipe_read_fd  = -1;
    int      pipe_write_fd = -1;
    int      saved_stdout  = -1;
    int      saved_stderr  = -1;
    bool     capturing_stdout = false;
    bool     capturing_stderr = false;
    std::string buffer;
    std::mutex  mtx;

    bool begin_stdout() {
        std::lock_guard<std::mutex> lk(mtx);
        if (capturing_stdout) return false;
        int fds[2];
        if (pipe(fds) != 0) { fprintf(stderr, "[DEBUG] pipe() failed: errno=%d\n", errno); fflush(stderr); return false; }
        pipe_read_fd  = fds[0];
        pipe_write_fd = fds[1];
        fflush(stdout);
        saved_stdout = dup(STDOUT_FILENO);
        if (saved_stdout < 0) { close(fds[0]); close(fds[1]); fprintf(stderr, "[DEBUG] dup failed\n"); fflush(stderr); return false; }
        int rc = dup2(pipe_write_fd, STDOUT_FILENO);
        close(pipe_write_fd);
        pipe_write_fd = -1;
        capturing_stdout = true;
        buffer.clear();
        return true;
    }

    bool begin_stderr() {
        std::lock_guard<std::mutex> lk(mtx);
        if (capturing_stderr) return false;
        int fds[2];
        if (pipe(fds) != 0) { fprintf(stderr, "[DEBUG] pipe() failed: errno=%d\n", errno); fflush(stderr); return false; }
        pipe_read_fd  = fds[0];
        pipe_write_fd = fds[1];
        fflush(stderr);
        saved_stderr = dup(STDERR_FILENO);
        if (saved_stderr < 0) { close(fds[0]); close(fds[1]); fprintf(stderr, "[DEBUG] dup failed\n"); fflush(stderr); return false; }
        int rc = dup2(pipe_write_fd, STDERR_FILENO);
        close(pipe_write_fd);
        pipe_write_fd = -1;
        capturing_stderr = true;
        buffer.clear();
        return true;
    }

    // Flush everything in the pipe into buffer, restore the original fd,
    // and return all captured text so far.
    std::string end_capture() {
        std::lock_guard<std::mutex> lk(mtx);
        if (!capturing_stdout && !capturing_stderr) return buffer;


        close(pipe_write_fd);
        pipe_write_fd = -1;

      if (capturing_stdout) {
          dup2(saved_stdout, STDOUT_FILENO);
          close(saved_stdout);
          saved_stdout = -1;
          capturing_stdout = false;
      }
      if (capturing_stderr) {
          dup2(saved_stderr, STDERR_FILENO);
          close(saved_stderr);
          saved_stderr = -1;
          capturing_stderr = false;
      }

      // Now drain the pipe
       fflush(stderr);
      char chunk[4096];
      ssize_t n;
      while ((n = read(pipe_read_fd, chunk, sizeof(chunk))) > 0) {
          buffer.append(chunk, static_cast<size_t>(n));
      }
      close(pipe_read_fd);
      pipe_read_fd = -1;

        if (capturing_stdout) {
            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdout);
            saved_stdout = -1;
            capturing_stdout = false;
        }
        if (capturing_stderr) {
            dup2(saved_stderr, STDERR_FILENO);
            close(saved_stderr);
            saved_stderr = -1;
            capturing_stderr = false;
        }

        if (capturing_stdout || true) {
            fwrite(buffer.c_str(), 1, buffer.size(), stdout);
            fflush(stdout);
        }

        return buffer;
    }

    // Drain whatever has been written so far WITHOUT ending capture.
    // Useful for intermediate checks.
    std::string peek() {
        std::lock_guard<std::mutex> lk(mtx);
        if (!capturing_stdout && !capturing_stderr) return buffer;


        int flags = fcntl(pipe_read_fd, F_GETFL, 0);
        fcntl(pipe_read_fd, F_SETFL, flags | O_NONBLOCK);
        char chunk[4096];
        ssize_t n;
        while ((n = read(pipe_read_fd, chunk, sizeof(chunk))) > 0) {
            buffer.append(chunk, static_cast<size_t>(n));
        }
        fcntl(pipe_read_fd, F_SETFL, flags); // restore
        return buffer;
    }

    void write_stdout(const std::string &msg) {
        // Write to the real stdout even when capturing
        int target = (capturing_stdout && saved_stdout >= 0) ? saved_stdout : STDOUT_FILENO;
        write(target, msg.c_str(), msg.size());
    }

    void write_stderr(const std::string &msg) {
        int target = (capturing_stderr && saved_stderr >= 0) ? saved_stderr : STDERR_FILENO;
        write(target, msg.c_str(), msg.size());
    }
};

using StdoutCaptureSP = std::shared_ptr<StdoutCapture>;

// ── Helpers ───────────────────────────────────────────────────────────────────

static void pack_nil(FalconResultSlot *out, int32_t *oc) {
    out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}
static void pack_capture(StdoutCaptureSP p, FalconResultSlot *out, int32_t *oc) {
    out[0] = {}; out[0].tag = FALCON_TYPE_OPAQUE;
    out[0].value.opaque.type_name = "IOCapture";
    out[0].value.opaque.ptr       = new StdoutCaptureSP(std::move(p));
    out[0].value.opaque.deleter   = [](void *q){ delete static_cast<StdoutCaptureSP*>(q); };
    *oc = 1;
}

// ── extern "C" bindings ───────────────────────────────────────────────────────

extern "C" {

// IOCapture.New() -> (IOCapture cap)
void STRUCTIOCaptureNew(const FalconParamEntry *p, int32_t pc,
                        FalconResultSlot *out, int32_t *oc) {
    (void)p; (void)pc;
    pack_capture(std::make_shared<StdoutCapture>(), out, oc);
}

// IOCapture.BeginStdout() -> (Error err)
void STRUCTIOCaptureBeginStdout(const FalconParamEntry *p, int32_t pc,
                                FalconResultSlot *out, int32_t *oc) {
    get_opaque<StdoutCapture>(p, pc, "this")->begin_stdout();
    pack_nil(out, oc);
}

// IOCapture.BeginStderr() -> (Error err)
void STRUCTIOCaptureBeginStderr(const FalconParamEntry *p, int32_t pc,
                                FalconResultSlot *out, int32_t *oc) {
    get_opaque<StdoutCapture>(p, pc, "this")->begin_stderr();
    pack_nil(out, oc);
}

// IOCapture.End() -> (string captured)
// Ends capture, returns everything captured so far, and tees it to the real fd.
void STRUCTIOCaptureEnd(const FalconParamEntry *p, int32_t pc,
                        FalconResultSlot *out, int32_t *oc) {
    std::string result = get_opaque<StdoutCapture>(p, pc, "this")->end_capture();
    pack_results(FunctionResult{result}, out, 16, oc);
}

// IOCapture.Peek() -> (string captured)
// Non-destructively reads whatever has landed in the buffer so far.
void STRUCTIOCapturePeek(const FalconParamEntry *p, int32_t pc,
                         FalconResultSlot *out, int32_t *oc) {
    std::string result = get_opaque<StdoutCapture>(p, pc, "this")->peek();
    pack_results(FunctionResult{result}, out, 16, oc);
}

// IOCapture.IsActive() -> (bool active)
void STRUCTIOCaptureIsActive(const FalconParamEntry *p, int32_t pc,
                             FalconResultSlot *out, int32_t *oc) {
    auto cap = get_opaque<StdoutCapture>(p, pc, "this");
    pack_results(FunctionResult{cap->capturing_stdout || cap->capturing_stderr},
                 out, 16, oc);
}

// IOCapture.WriteStdout(string msg) -> (Error err)
// Writes directly to the real stdout (bypassing any active capture pipe).
void STRUCTIOCaptureWriteStdout(const FalconParamEntry *p, int32_t pc,
                                FalconResultSlot *out, int32_t *oc) {
    auto pm  = unpack_params(p, pc);
    auto msg = std::get<std::string>(pm.at("msg"));
    get_opaque<StdoutCapture>(p, pc, "this")->write_stdout(msg);
    pack_nil(out, oc);
}

// IOCapture.WriteStderr(string msg) -> (Error err)
void STRUCTIOCaptureWriteStderr(const FalconParamEntry *p, int32_t pc,
                                FalconResultSlot *out, int32_t *oc) {
    auto pm  = unpack_params(p, pc);
    auto msg = std::get<std::string>(pm.at("msg"));
    get_opaque<StdoutCapture>(p, pc, "this")->write_stderr(msg);
    pack_nil(out, oc);
}

// Println(string msg) -> (Error err)
// Convenience top-level routine — writes msg + newline to stdout.
void Println(const FalconParamEntry *p, int32_t pc,
             FalconResultSlot *out, int32_t *oc) {
    auto pm  = unpack_params(p, pc);
    auto msg = std::get<std::string>(pm.at("msg"));
    std::cout << msg << "\n";
    std::cout.flush();
    pack_nil(out, oc);
}

// Print(string msg) -> (Error err)
void Print(const FalconParamEntry *p, int32_t pc,
           FalconResultSlot *out, int32_t *oc) {
    auto pm  = unpack_params(p, pc);
    auto msg = std::get<std::string>(pm.at("msg"));
    std::cout << msg;
    std::cout.flush();
    pack_nil(out, oc);
}

// Eprintln(string msg) -> (Error err)
void Eprintln(const FalconParamEntry *p, int32_t pc,
              FalconResultSlot *out, int32_t *oc) {
    auto pm  = unpack_params(p, pc);
    auto msg = std::get<std::string>(pm.at("msg"));
    std::cerr << msg << "\n";
    std::cerr.flush();
    pack_nil(out, oc);
}

// Eprint(string msg) -> (Error err)
void Eprint(const FalconParamEntry *p, int32_t pc,
            FalconResultSlot *out, int32_t *oc) {
    auto pm  = unpack_params(p, pc);
    auto msg = std::get<std::string>(pm.at("msg"));
    std::cerr << msg;
    std::cerr.flush();
    pack_nil(out, oc);
}

} // extern "C"
