#include <falcon-typing/FFIHelpers.hpp>
#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

// ── TestContext ───────────────────────────────────────────────────────────────
struct FalconTestContext {
    std::string name;
    std::vector<std::string> failures;
    std::vector<std::string> logs;

    bool passed() const { return failures.empty(); }
    int  failure_count() const { return static_cast<int>(failures.size()); }

    void expect(bool cond, const std::string& msg) {
        if (!cond) failures.push_back("[FAIL] " + name + ": " + msg);
    }
    void fail(const std::string& msg) {
        failures.push_back("[FAIL] " + name + ": " + msg);
    }
    void log(const std::string& msg) {
        logs.push_back("[LOG]  " + name + ": " + msg);
        std::cout << "[LOG]  " << name << ": " << msg << "\n";
    }
};
using FalconTestContextSP = std::shared_ptr<FalconTestContext>;

// ── TestSuite ─────────────────────────────────────────────────────────────────
struct FalconTestSuite {
    std::string name;
    std::vector<FalconTestContextSP> results;

    int total()  const { return static_cast<int>(results.size()); }
    int passed() const {
        int n = 0;
        for (auto& r : results) if (r->passed()) ++n;
        return n;
    }
    int failed() const { return total() - passed(); }

    void print_summary() const {
        std::cout << "\n=== Test Suite: " << name << " ===\n";
        for (auto& r : results) {
            if (r->passed()) {
                std::cout << "  [PASS] " << r->name << "\n";
            } else {
                std::cout << "  [FAIL] " << r->name << "\n";
                for (auto& f : r->failures) std::cout << "         " << f << "\n";
            }
        }
        std::cout << "---\n";
        std::cout << "  Total:  " << total()  << "\n";
        std::cout << "  Passed: " << passed() << "\n";
        std::cout << "  Failed: " << failed() << "\n";
        std::cout << "=========================\n\n";
    }
};
using FalconTestSuiteSP = std::shared_ptr<FalconTestSuite>;

// ── Helpers ───────────────────────────────────────────────────────────────────
static void pack_opaque_ctx(FalconTestContextSP ctx, FalconResultSlot* out, int32_t* out_count) {
    out[0] = {};
    out[0].tag = FALCON_TYPE_OPAQUE;
    out[0].value.opaque.type_name = "FalconTestContext";
    out[0].value.opaque.ptr = new FalconTestContextSP(std::move(ctx));
    out[0].value.opaque.deleter = [](void* p) { delete static_cast<FalconTestContextSP*>(p); };
    *out_count = 1;
}

static void pack_opaque_suite(FalconTestSuiteSP suite, FalconResultSlot* out, int32_t* out_count) {
    out[0] = {};
    out[0].tag = FALCON_TYPE_OPAQUE;
    out[0].value.opaque.type_name = "FalconTestSuite";
    out[0].value.opaque.ptr = new FalconTestSuiteSP(std::move(suite));
    out[0].value.opaque.deleter = [](void* p) { delete static_cast<FalconTestSuiteSP*>(p); };
    *out_count = 1;
}

static void pack_nil_error(FalconResultSlot* out, int32_t* out_count) {
    out[0] = {};
    out[0].tag = FALCON_TYPE_NIL;
    *out_count = 1;
}

// ── TestContext extern "C" exports ────────────────────────────────────────────
extern "C" {

void STRUCTTestContextNew(const FalconParamEntry* params, int32_t param_count,
                          FalconResultSlot* out, int32_t* out_count) {
    auto pm = unpack_params(params, param_count);
    std::string name = std::get<std::string>(pm.at("name"));
    auto ctx = std::make_shared<FalconTestContext>();
    ctx->name = name;
    pack_opaque_ctx(std::move(ctx), out, out_count);
}

void STRUCTTestContextName(const FalconParamEntry* params, int32_t param_count,
                           FalconResultSlot* out, int32_t* out_count) {
    auto ctx = get_opaque<FalconTestContext>(params, param_count, "this");
    pack_results(FunctionResult{ctx->name}, out, 16, out_count);
}

void STRUCTTestContextPassed(const FalconParamEntry* params, int32_t param_count,
                              FalconResultSlot* out, int32_t* out_count) {
    auto ctx = get_opaque<FalconTestContext>(params, param_count, "this");
    pack_results(FunctionResult{ctx->passed()}, out, 16, out_count);
}

void STRUCTTestContextFailureCount(const FalconParamEntry* params, int32_t param_count,
                                    FalconResultSlot* out, int32_t* out_count) {
    auto ctx = get_opaque<FalconTestContext>(params, param_count, "this");
    pack_results(FunctionResult{(int64_t)ctx->failure_count()}, out, 16, out_count);
}

void STRUCTTestContextExpectTrue(const FalconParamEntry* params, int32_t param_count,
                                  FalconResultSlot* out, int32_t* out_count) {
    auto pm = unpack_params(params, param_count);
    auto ctx = get_opaque<FalconTestContext>(params, param_count, "this");
    bool condition = std::get<bool>(pm.at("condition"));
    std::string msg = std::get<std::string>(pm.at("msg"));
    ctx->expect(condition, msg);
    pack_nil_error(out, out_count);
}

void STRUCTTestContextExpectFalse(const FalconParamEntry* params, int32_t param_count,
                                   FalconResultSlot* out, int32_t* out_count) {
    auto pm = unpack_params(params, param_count);
    auto ctx = get_opaque<FalconTestContext>(params, param_count, "this");
    bool condition = std::get<bool>(pm.at("condition"));
    std::string msg = std::get<std::string>(pm.at("msg"));
    ctx->expect(!condition, msg);
    pack_nil_error(out, out_count);
}

void STRUCTTestContextExpectIntEq(const FalconParamEntry* params, int32_t param_count,
                                   FalconResultSlot* out, int32_t* out_count) {
    auto pm = unpack_params(params, param_count);
    auto ctx = get_opaque<FalconTestContext>(params, param_count, "this");
    int64_t expected = std::get<int64_t>(pm.at("expected"));
    int64_t actual   = std::get<int64_t>(pm.at("actual"));
    std::string msg  = std::get<std::string>(pm.at("msg"));
    if (expected != actual) {
        std::ostringstream oss;
        oss << msg << " (expected=" << expected << " actual=" << actual << ")";
        ctx->fail(oss.str());
    }
    pack_nil_error(out, out_count);
}

void STRUCTTestContextExpectIntNe(const FalconParamEntry* params, int32_t param_count,
                                   FalconResultSlot* out, int32_t* out_count) {
    auto pm = unpack_params(params, param_count);
    auto ctx = get_opaque<FalconTestContext>(params, param_count, "this");
    int64_t expected = std::get<int64_t>(pm.at("expected"));
    int64_t actual   = std::get<int64_t>(pm.at("actual"));
    std::string msg  = std::get<std::string>(pm.at("msg"));
    if (expected == actual) {
        std::ostringstream oss;
        oss << msg << " (both=" << expected << ", expected them to differ)";
        ctx->fail(oss.str());
    }
    pack_nil_error(out, out_count);
}

void STRUCTTestContextExpectIntLt(const FalconParamEntry* params, int32_t param_count,
                                   FalconResultSlot* out, int32_t* out_count) {
    auto pm = unpack_params(params, param_count);
    auto ctx = get_opaque<FalconTestContext>(params, param_count, "this");
    int64_t a   = std::get<int64_t>(pm.at("a"));
    int64_t b   = std::get<int64_t>(pm.at("b"));
    std::string msg = std::get<std::string>(pm.at("msg"));
    if (!(a < b)) {
        std::ostringstream oss;
        oss << msg << " (expected " << a << " < " << b << ")";
        ctx->fail(oss.str());
    }
    pack_nil_error(out, out_count);
}

void STRUCTTestContextExpectIntGt(const FalconParamEntry* params, int32_t param_count,
                                   FalconResultSlot* out, int32_t* out_count) {
    auto pm = unpack_params(params, param_count);
    auto ctx = get_opaque<FalconTestContext>(params, param_count, "this");
    int64_t a   = std::get<int64_t>(pm.at("a"));
    int64_t b   = std::get<int64_t>(pm.at("b"));
    std::string msg = std::get<std::string>(pm.at("msg"));
    if (!(a > b)) {
        std::ostringstream oss;
        oss << msg << " (expected " << a << " > " << b << ")";
        ctx->fail(oss.str());
    }
    pack_nil_error(out, out_count);
}

void STRUCTTestContextExpectFloatEq(const FalconParamEntry* params, int32_t param_count,
                                     FalconResultSlot* out, int32_t* out_count) {
    auto pm = unpack_params(params, param_count);
    auto ctx = get_opaque<FalconTestContext>(params, param_count, "this");
    double expected = std::get<double>(pm.at("expected"));
    double actual   = std::get<double>(pm.at("actual"));
    double tol      = std::get<double>(pm.at("tol"));
    std::string msg = std::get<std::string>(pm.at("msg"));
    if (std::fabs(expected - actual) > tol) {
        std::ostringstream oss;
        oss << msg << " (expected=" << expected << " actual=" << actual
            << " tol=" << tol << ")";
        ctx->fail(oss.str());
    }
    pack_nil_error(out, out_count);
}

void STRUCTTestContextExpectStrEq(const FalconParamEntry* params, int32_t param_count,
                                   FalconResultSlot* out, int32_t* out_count) {
    auto pm = unpack_params(params, param_count);
    auto ctx = get_opaque<FalconTestContext>(params, param_count, "this");
    std::string expected = std::get<std::string>(pm.at("expected"));
    std::string actual   = std::get<std::string>(pm.at("actual"));
    std::string msg      = std::get<std::string>(pm.at("msg"));
    if (expected != actual) {
        std::ostringstream oss;
        oss << msg << " (expected=\"" << expected << "\" actual=\"" << actual << "\")";
        ctx->fail(oss.str());
    }
    pack_nil_error(out, out_count);
}

void STRUCTTestContextExpectStrNe(const FalconParamEntry* params, int32_t param_count,
                                   FalconResultSlot* out, int32_t* out_count) {
    auto pm = unpack_params(params, param_count);
    auto ctx = get_opaque<FalconTestContext>(params, param_count, "this");
    std::string expected = std::get<std::string>(pm.at("expected"));
    std::string actual   = std::get<std::string>(pm.at("actual"));
    std::string msg      = std::get<std::string>(pm.at("msg"));
    if (expected == actual) {
        std::ostringstream oss;
        oss << msg << " (both=\"" << expected << "\", expected them to differ)";
        ctx->fail(oss.str());
    }
    pack_nil_error(out, out_count);
}

void STRUCTTestContextFail(const FalconParamEntry* params, int32_t param_count,
                            FalconResultSlot* out, int32_t* out_count) {
    auto pm = unpack_params(params, param_count);
    auto ctx = get_opaque<FalconTestContext>(params, param_count, "this");
    std::string msg = std::get<std::string>(pm.at("msg"));
    ctx->fail(msg);
    pack_nil_error(out, out_count);
}

void STRUCTTestContextLog(const FalconParamEntry* params, int32_t param_count,
                           FalconResultSlot* out, int32_t* out_count) {
    auto pm = unpack_params(params, param_count);
    auto ctx = get_opaque<FalconTestContext>(params, param_count, "this");
    std::string msg = std::get<std::string>(pm.at("msg"));
    ctx->log(msg);
    pack_nil_error(out, out_count);
}

// ── TestSuite extern "C" exports ──────────────────────────────────────────────

void STRUCTTestSuiteNew(const FalconParamEntry* params, int32_t param_count,
                         FalconResultSlot* out, int32_t* out_count) {
    auto pm = unpack_params(params, param_count);
    std::string name = std::get<std::string>(pm.at("suite_name"));
    auto suite = std::make_shared<FalconTestSuite>();
    suite->name = name;
    pack_opaque_suite(std::move(suite), out, out_count);
}

void STRUCTTestSuiteAddResult(const FalconParamEntry* params, int32_t param_count,
                               FalconResultSlot* out, int32_t* out_count) {
    auto suite = get_opaque<FalconTestSuite>(params, param_count, "this");
    auto ctx   = get_opaque<FalconTestContext>(params, param_count, "ctx");
    suite->results.push_back(ctx);
    pack_nil_error(out, out_count);
}

void STRUCTTestSuiteTotalCount(const FalconParamEntry* params, int32_t param_count,
                                FalconResultSlot* out, int32_t* out_count) {
    auto suite = get_opaque<FalconTestSuite>(params, param_count, "this");
    pack_results(FunctionResult{(int64_t)suite->total()}, out, 16, out_count);
}

void STRUCTTestSuitePassedCount(const FalconParamEntry* params, int32_t param_count,
                                 FalconResultSlot* out, int32_t* out_count) {
    auto suite = get_opaque<FalconTestSuite>(params, param_count, "this");
    pack_results(FunctionResult{(int64_t)suite->passed()}, out, 16, out_count);
}

void STRUCTTestSuiteFailedCount(const FalconParamEntry* params, int32_t param_count,
                                 FalconResultSlot* out, int32_t* out_count) {
    auto suite = get_opaque<FalconTestSuite>(params, param_count, "this");
    pack_results(FunctionResult{(int64_t)suite->failed()}, out, 16, out_count);
}

void STRUCTTestSuitePrintSummary(const FalconParamEntry* params, int32_t param_count,
                                  FalconResultSlot* out, int32_t* out_count) {
    auto suite = get_opaque<FalconTestSuite>(params, param_count, "this");
    suite->print_summary();
    pack_nil_error(out, out_count);
}

} // extern "C"
