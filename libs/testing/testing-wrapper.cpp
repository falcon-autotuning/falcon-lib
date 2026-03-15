#include <cmath>
#include <falcon-typing/FFIHelpers.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

static const char *C_GREEN = "\033[32m";
static const char *C_RED   = "\033[31m";
static const char *C_CYAN  = "\033[36m";
static const char *C_BOLD  = "\033[1m";
static const char *C_RESET = "\033[0m";

// ── TestContext ────────────────────────────────────────────────────────────
struct TestContext {
  std::string name;
  std::vector<std::string> failures;

  bool passed()        const { return failures.empty(); }
  int  failure_count() const { return static_cast<int>(failures.size()); }
  void expect(bool ok, const std::string &msg) { if (!ok) failures.push_back(msg); }
  void fail(const std::string &msg)            { failures.push_back(msg); }
  void log(const std::string &msg) {
    std::cout << C_CYAN << "[ INFO     ] " << C_RESET
              << name << ": " << msg << "\n";
  }
};
using TestContextSP = std::shared_ptr<TestContext>;

// ── TestRunner ─────────────────────────────────────────────────────────────
struct TestRunner {
  std::string                            suite_name;
  std::vector<std::string>              test_names;
  std::vector<bool>                     results;
  std::vector<std::vector<std::string>> failure_msgs;
  int current_idx = 0;

  int  total()    const { return static_cast<int>(test_names.size()); }
  bool has_next() const { return current_idx < total(); }
  int  passed()   const { int n=0; for (auto b:results) if(b) ++n; return n; }
  int  failed()   const { return total() - passed(); }

  std::string current_name() const {
    return (current_idx < total()) ? test_names[current_idx] : "";
  }

  void register_test(const std::string &name) { test_names.push_back(name); }

  void print_header() const {
    std::cout << C_BOLD << "[==========] " << C_RESET
              << "Running " << total() << " test(s) from " << suite_name << "\n"
              << C_GREEN << "[----------] " << C_RESET
              << "Global test environment set-up.\n";
  }

  TestContextSP begin_current() {
    std::cout << C_GREEN << "[ RUN      ] " << C_RESET
              << suite_name << "." << current_name() << "\n";
    auto ctx  = std::make_shared<TestContext>();
    ctx->name = current_name();
    return ctx;
  }

  void end_current(TestContextSP ctx) {
    bool ok = ctx->passed();
    results.push_back(ok);
    failure_msgs.push_back(ctx->failures);
    if (ok) {
      std::cout << C_GREEN << "[       OK ] " << C_RESET
                << suite_name << "." << ctx->name << "\n";
    } else {
      for (auto &f : ctx->failures)
        std::cout << "             " << C_RED << "✗ " << f << C_RESET << "\n";
      std::cout << C_RED << "[  FAILED  ] " << C_RESET
                << suite_name << "." << ctx->name << "\n";
    }
    ++current_idx;
  }

  void print_summary() const {
    std::cout << "\n" << C_BOLD << "[==========] " << C_RESET
              << total() << " test(s) from " << suite_name << "\n"
              << C_GREEN << "[----------] " << C_RESET
              << "Global test environment tear-down.\n";
    for (int i = 0; i < (int)test_names.size(); ++i) {
      if (results[i])
        std::cout << C_GREEN << "[  PASSED  ] " << C_RESET << test_names[i] << "\n";
      else {
        std::cout << C_RED << "[  FAILED  ] " << C_RESET << test_names[i] << "\n";
        for (auto &f : failure_msgs[i])
          std::cout << "             " << C_RED << "✗ " << f << C_RESET << "\n";
      }
    }
    std::cout << C_BOLD << "[----------]\n" << C_RESET;
    if (passed() > 0)
      std::cout << C_GREEN << "[  PASSED  ] " << C_RESET << passed() << " test(s)\n";
    if (failed() > 0)
      std::cout << C_RED   << "[  FAILED  ] " << C_RESET << failed() << " test(s)\n";
    std::cout << C_BOLD << "[==========] " << C_RESET << total() << " test(s) ran.\n\n";
  }
};
using TestRunnerSP = std::shared_ptr<TestRunner>;

// ── Pack helpers ───────────────────────────────────────────────────────────
static void pack_nil(FalconResultSlot *out, int32_t *oc) {
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}
static void pack_ctx(TestContextSP p, FalconResultSlot *out, int32_t *oc) {
  out[0] = {}; out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "TestContext";
  out[0].value.opaque.ptr = new TestContextSP(std::move(p));
  out[0].value.opaque.deleter = [](void *q){ delete static_cast<TestContextSP*>(q); };
  *oc = 1;
}
static void pack_runner(TestRunnerSP p, FalconResultSlot *out, int32_t *oc) {
  out[0] = {}; out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "TestRunner";
  out[0].value.opaque.ptr = new TestRunnerSP(std::move(p));
  out[0].value.opaque.deleter = [](void *q){ delete static_cast<TestRunnerSP*>(q); };
  *oc = 1;
}

extern "C" {

// ── TestContext ────────────────────────────────────────────────────────────
void STRUCTTestContextNew(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto ctx = std::make_shared<TestContext>();
  ctx->name = std::get<std::string>(pm.at("name"));
  pack_ctx(std::move(ctx), out, oc);
}
void STRUCTTestContextName(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{get_opaque<TestContext>(p,pc,"this")->name}, out, 16, oc);
}
void STRUCTTestContextPassed(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{get_opaque<TestContext>(p,pc,"this")->passed()}, out, 16, oc);
}
void STRUCTTestContextFailureCount(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{(int64_t)get_opaque<TestContext>(p,pc,"this")->failure_count()}, out, 16, oc);
}
void STRUCTTestContextExpectTrue(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p,pc);
  get_opaque<TestContext>(p,pc,"this")->expect(std::get<bool>(pm.at("condition")), std::get<std::string>(pm.at("msg")));
  pack_nil(out, oc);
}
void STRUCTTestContextExpectFalse(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p,pc);
  get_opaque<TestContext>(p,pc,"this")->expect(!std::get<bool>(pm.at("condition")), std::get<std::string>(pm.at("msg")));
  pack_nil(out, oc);
}
void STRUCTTestContextExpectIntEq(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p,pc); auto ctx = get_opaque<TestContext>(p,pc,"this");
  int64_t e=std::get<int64_t>(pm.at("expected")), a=std::get<int64_t>(pm.at("actual"));
  std::string m=std::get<std::string>(pm.at("msg"));
  if (e!=a) { std::ostringstream s; s<<m<<" (expected="<<e<<" actual="<<a<<")"; ctx->fail(s.str()); }
  pack_nil(out, oc);
}
void STRUCTTestContextExpectIntNe(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p,pc); auto ctx = get_opaque<TestContext>(p,pc,"this");
  int64_t e=std::get<int64_t>(pm.at("expected")), a=std::get<int64_t>(pm.at("actual"));
  std::string m=std::get<std::string>(pm.at("msg"));
  if (e==a) { std::ostringstream s; s<<m<<" (both="<<e<<", expected to differ)"; ctx->fail(s.str()); }
  pack_nil(out, oc);
}
void STRUCTTestContextExpectIntLt(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p,pc); auto ctx = get_opaque<TestContext>(p,pc,"this");
  int64_t a=std::get<int64_t>(pm.at("a")), b=std::get<int64_t>(pm.at("b"));
  std::string m=std::get<std::string>(pm.at("msg"));
  if (!(a<b)) { std::ostringstream s; s<<m<<" (expected "<<a<<" < "<<b<<")"; ctx->fail(s.str()); }
  pack_nil(out, oc);
}
void STRUCTTestContextExpectIntGt(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p,pc); auto ctx = get_opaque<TestContext>(p,pc,"this");
  int64_t a=std::get<int64_t>(pm.at("a")), b=std::get<int64_t>(pm.at("b"));
  std::string m=std::get<std::string>(pm.at("msg"));
  if (!(a>b)) { std::ostringstream s; s<<m<<" (expected "<<a<<" > "<<b<<")"; ctx->fail(s.str()); }
  pack_nil(out, oc);
}
void STRUCTTestContextExpectFloatEq(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p,pc); auto ctx = get_opaque<TestContext>(p,pc,"this");
  double e=std::get<double>(pm.at("expected")), a=std::get<double>(pm.at("actual")), tol=std::get<double>(pm.at("tol"));
  std::string m=std::get<std::string>(pm.at("msg"));
  if (std::fabs(e-a)>tol) { std::ostringstream s; s<<m<<" (expected="<<e<<" actual="<<a<<" tol="<<tol<<")"; ctx->fail(s.str()); }
  pack_nil(out, oc);
}
void STRUCTTestContextExpectStrEq(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p,pc); auto ctx = get_opaque<TestContext>(p,pc,"this");
  std::string e=std::get<std::string>(pm.at("expected")), a=std::get<std::string>(pm.at("actual")), m=std::get<std::string>(pm.at("msg"));
  if (e!=a) { std::ostringstream s; s<<m<<" (expected=\""<<e<<"\" actual=\""<<a<<"\")"; ctx->fail(s.str()); }
  pack_nil(out, oc);
}
void STRUCTTestContextExpectStrNe(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p,pc); auto ctx = get_opaque<TestContext>(p,pc,"this");
  std::string e=std::get<std::string>(pm.at("expected")), a=std::get<std::string>(pm.at("actual")), m=std::get<std::string>(pm.at("msg"));
  if (e==a) { std::ostringstream s; s<<m<<" (both=\""<<e<<"\", expected to differ)"; ctx->fail(s.str()); }
  pack_nil(out, oc);
}
void STRUCTTestContextFail(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p,pc);
  get_opaque<TestContext>(p,pc,"this")->fail(std::get<std::string>(pm.at("msg")));
  pack_nil(out, oc);
}
void STRUCTTestContextLog(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p,pc);
  get_opaque<TestContext>(p,pc,"this")->log(std::get<std::string>(pm.at("msg")));
  pack_nil(out, oc);
}

// ── TestRunner ────────────────────────────────────────────────────────────
void STRUCTTestRunnerNew(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p,pc);
  auto r = std::make_shared<TestRunner>();
  r->suite_name = std::get<std::string>(pm.at("suite_name"));
  pack_runner(std::move(r), out, oc);
}
void STRUCTTestRunnerRegister(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p,pc);
  get_opaque<TestRunner>(p,pc,"this")->register_test(std::get<std::string>(pm.at("test_name")));
  pack_nil(out, oc);
}
void STRUCTTestRunnerPrintHeader(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  get_opaque<TestRunner>(p,pc,"this")->print_header();
  pack_nil(out, oc);
}
void STRUCTTestRunnerBeginCurrent(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  pack_ctx(get_opaque<TestRunner>(p,pc,"this")->begin_current(), out, oc);
}
void STRUCTTestRunnerEndCurrent(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  get_opaque<TestRunner>(p,pc,"this")->end_current(get_opaque<TestContext>(p,pc,"ctx"));
  pack_nil(out, oc);
}
void STRUCTTestRunnerHasNext(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{get_opaque<TestRunner>(p,pc,"this")->has_next()}, out, 16, oc);
}
void STRUCTTestRunnerCurrentName(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{get_opaque<TestRunner>(p,pc,"this")->current_name()}, out, 16, oc);
}
void STRUCTTestRunnerPrintSummary(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  get_opaque<TestRunner>(p,pc,"this")->print_summary();
  pack_nil(out, oc);
}
void STRUCTTestRunnerPassedCount(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{(int64_t)get_opaque<TestRunner>(p,pc,"this")->passed()}, out, 16, oc);
}
void STRUCTTestRunnerFailedCount(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{(int64_t)get_opaque<TestRunner>(p,pc,"this")->failed()}, out, 16, oc);
}

} // extern "C"
