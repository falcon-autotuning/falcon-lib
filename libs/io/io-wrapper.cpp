#include <atomic>
#include <cstdio>
#include <falcon-typing/FFIHelpers.hpp>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

// ── StdoutCapture
// ───────────────────────────────────────────────────────────── Redirects
// stdout to an internal pipe so that lines written by any code (Falcon or C++)
// can be intercepted, buffered, and then forwarded to the original stdout.  The
// captured text remains accessible via Drain().
//
// Thread-safety: a single mutex guards the internal buffer and the dup2 state.
// Only one capture may be active at a time per process.

struct StdoutCapture {
  int pipe_read_fd = -1;
  int pipe_write_fd = -1;
  int saved_stdout = -1;
  int saved_stderr = -1;
  bool capturing_stdout = false;
  bool capturing_stderr = false;
  std::string buffer;
  std::mutex mtx;

  bool begin_stdout() {
    std::lock_guard<std::mutex> lk(mtx);
    if (capturing_stdout)
      return false;
    int fds[2];
    if (pipe(fds) != 0) {
      fprintf(stderr, "[DEBUG] pipe() failed: errno=%d\n", errno);
      fflush(stderr);
      return false;
    }
    pipe_read_fd = fds[0];
    pipe_write_fd = fds[1];
    fflush(stdout);
    saved_stdout = dup(STDOUT_FILENO);
    if (saved_stdout < 0) {
      close(fds[0]);
      close(fds[1]);
      fprintf(stderr, "[DEBUG] dup failed\n");
      fflush(stderr);
      return false;
    }
    int rc = dup2(pipe_write_fd, STDOUT_FILENO);
    close(pipe_write_fd);
    pipe_write_fd = -1;
    capturing_stdout = true;
    buffer.clear();
    return true;
  }

  bool begin_stderr() {
    std::lock_guard<std::mutex> lk(mtx);
    if (capturing_stderr)
      return false;
    int fds[2];
    if (pipe(fds) != 0) {
      fprintf(stderr, "[DEBUG] pipe() failed: errno=%d\n", errno);
      fflush(stderr);
      return false;
    }
    pipe_read_fd = fds[0];
    pipe_write_fd = fds[1];
    fflush(stderr);
    saved_stderr = dup(STDERR_FILENO);
    if (saved_stderr < 0) {
      close(fds[0]);
      close(fds[1]);
      fprintf(stderr, "[DEBUG] dup failed\n");
      fflush(stderr);
      return false;
    }
    int rc = dup2(pipe_write_fd, STDERR_FILENO);
    close(pipe_write_fd);
    pipe_write_fd = -1;
    capturing_stderr = true;
    buffer.clear();
    return true;
  }

  std::string end_capture() {
    std::lock_guard<std::mutex> lk(mtx);
    if (!capturing_stdout && !capturing_stderr)
      return buffer;

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

    fflush(stderr);
    char chunk[4096];
    ssize_t n;
    while ((n = read(pipe_read_fd, chunk, sizeof(chunk))) > 0) {
      buffer.append(chunk, static_cast<size_t>(n));
    }
    close(pipe_read_fd);
    pipe_read_fd = -1;

    if (capturing_stdout || true) {
      fwrite(buffer.c_str(), 1, buffer.size(), stdout);
      fflush(stdout);
    }

    return buffer;
  }

  std::string peek() {
    std::lock_guard<std::mutex> lk(mtx);
    if (!capturing_stdout && !capturing_stderr)
      return buffer;

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
    int target =
        (capturing_stdout && saved_stdout >= 0) ? saved_stdout : STDOUT_FILENO;
    write(target, msg.c_str(), msg.size());
  }

  void write_stderr(const std::string &msg) {
    int target =
        (capturing_stderr && saved_stderr >= 0) ? saved_stderr : STDERR_FILENO;
    write(target, msg.c_str(), msg.size());
  }
};

using StdoutCaptureSP = std::shared_ptr<StdoutCapture>;

namespace {
std::shared_ptr<StdoutCapture> g_capture;
std::once_flag g_cap_init_flag;

std::shared_ptr<StdoutCapture> get_global_capture() {
  std::call_once(g_cap_init_flag,
                 [] { g_capture = std::make_shared<StdoutCapture>(); });
  return g_capture;
}
} // namespace

// ── Helpers
// ───────────────────────────────────────────────────────────────────

static void pack_nil(FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// ── extern "C" bindings
// ───────────────────────────────────────────────────────

extern "C" {

// ── IOCapture
// ─────────────────────────────────────────────────────────────────

void BeginStdout(const FalconParamEntry *p, int32_t pc,
                                FalconResultSlot *out, int32_t *oc) {
  get_global_capture()->begin_stdout();
  pack_nil(out, oc);
}

void BeginStderr(const FalconParamEntry *p, int32_t pc,
                                FalconResultSlot *out, int32_t *oc) {
  get_global_capture()->begin_stderr();
  pack_nil(out, oc);
}

void End(const FalconParamEntry *p, int32_t pc,
                        FalconResultSlot *out, int32_t *oc) {
  std::string result = get_global_capture()->end_capture();
  pack_results(FunctionResult{result}, out, 16, oc);
}

void Peek(const FalconParamEntry *p, int32_t pc,
                         FalconResultSlot *out, int32_t *oc) {
  std::string result = get_global_capture()->peek();
  pack_results(FunctionResult{result}, out, 16, oc);
}

void IsActive(const FalconParamEntry *p, int32_t pc,
                             FalconResultSlot *out, int32_t *oc) {
  auto cap = get_global_capture();
  pack_results(FunctionResult{cap->capturing_stdout || cap->capturing_stderr},
               out, 16, oc);
}

void WriteStdout(const FalconParamEntry *p, int32_t pc,
                                FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto msg = std::get<std::string>(pm.at("msg"));
  get_global_capture()->write_stdout(msg);
  pack_nil(out, oc);
}

void WriteStderr(const FalconParamEntry *p, int32_t pc,
                                FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto msg = std::get<std::string>(pm.at("msg"));
  get_global_capture()->write_stderr(msg);
  pack_nil(out, oc);
}

// ── IO
// ────────────────────────────────────────────────────────────────────────

void Println(const FalconParamEntry *p, int32_t pc,
                     FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto msg = std::get<std::string>(pm.at("msg"));
  std::cout << msg << "\n";
  std::cout.flush();
  pack_nil(out, oc);
}

void Print(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
                   int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto msg = std::get<std::string>(pm.at("msg"));
  std::cout << msg;
  std::cout.flush();
  pack_nil(out, oc);
}

void Eprintln(const FalconParamEntry *p, int32_t pc,
                      FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto msg = std::get<std::string>(pm.at("msg"));
  std::cerr << msg << "\n";
  std::cerr.flush();
  pack_nil(out, oc);
}

void Eprint(const FalconParamEntry *p, int32_t pc,
                    FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto msg = std::get<std::string>(pm.at("msg"));
  std::cerr << msg;
  std::cerr.flush();
  pack_nil(out, oc);
}

} // extern "C"
