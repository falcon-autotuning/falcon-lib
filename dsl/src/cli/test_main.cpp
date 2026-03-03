// falcon-test: Fixture-aware test runner for the Falcon DSL.
//
// Usage:
//   falcon-test <file.fal> [file2.fal ...] [--log-level L] [--dump] [--help]
//
// The tool reads one or more .fal test files.  Each file may contain any
// number of test suite autotuners.  A test suite autotuner has:
//   - Signature exactly:  autotuner Name -> (int passed, int failed)
//   - A  start -> __init;  entry (written by the user)
//   - Optional  state setup (TestRunner runner)       ending -> __begin(runner)
//   - Optional  state teardown (TestRunner runner, TestContext t)  ending -> __end(runner, t)
//   - One or more  state test_<name> (TestRunner runner, TestContext t)  bodies
//
// falcon-test injects the harness states (__init, __loop, __begin, __dispatch,
// __end, __finish) into each suite autotuner before loading.  Users never
// write or see these states.
//
// Imports in the user file are preserved verbatim so AutotunerEngine resolves
// them correctly from the original file's directory.
//
// Exit codes:
//   0  all tests in all suites passed
//   1  usage / argument error
//   2  file load / generation error
//   3  one or more tests failed

#include "falcon-dsl/AutotunerEngine.hpp"
#include "falcon-dsl/log.hpp"
#include <falcon-typing/PrimitiveTypes.hpp>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ── Helpers ────────────────────────────────────────────────────────────────

static std::string read_file(const std::string &path) {
  std::ifstream f(path);
  if (!f) throw std::runtime_error("Cannot open: " + path);
  return {std::istreambuf_iterator<char>(f), {}};
}

static void write_file(const std::string &path, const std::string &content) {
  std::ofstream f(path);
  if (!f) throw std::runtime_error("Cannot write: " + path);
  f << content;
}

// ── Suite descriptor ────────────────────────────────────────────────────────

struct SuiteInfo {
  std::string name;
  std::vector<std::string> test_names;   // all state test_* names (first state only for multi-step)
  bool has_setup    = false;
  bool has_teardown = false;
};

// ── Scan a source string for suite autotuners ────────────────────���──────────
// A suite autotuner matches:  autotuner <Name> -> (int passed, int failed)
// We do a character-level scan to extract the full body of each such autotuner
// so we can inject harness states into it.

struct AutotunerBlock {
  std::string name;
  size_t      open_brace;   // index of { in src
  size_t      close_brace;  // index of matching } in src
};

// Find the matching closing brace starting from open_pos (which points at '{').
static size_t find_matching_brace(const std::string &src, size_t open_pos) {
  int depth = 0;
  bool in_str = false;
  bool in_line_comment = false;
  bool in_block_comment = false;
  for (size_t i = open_pos; i < src.size(); ++i) {
    char c = src[i];
    // handle comments and strings to avoid false positives
    if (in_line_comment) {
      if (c == '\n') in_line_comment = false;
      continue;
    }
    if (in_block_comment) {
      if (c == '*' && i+1 < src.size() && src[i+1] == '/') { in_block_comment = false; ++i; }
      continue;
    }
    if (!in_str && c == '/' && i+1 < src.size()) {
      if (src[i+1] == '/') { in_line_comment = true; ++i; continue; }
      if (src[i+1] == '*') { in_block_comment = true; ++i; continue; }
    }
    if (!in_str && c == '"') { in_str = true; continue; }
    if (in_str) {
      if (c == '\\') { ++i; continue; } // skip escaped char
      if (c == '"')  { in_str = false; }
      continue;
    }
    if (c == '{') ++depth;
    if (c == '}') { --depth; if (depth == 0) return i; }
  }
  return std::string::npos;
}

// Extract all autotuner blocks that match the test-suite signature.
static std::vector<AutotunerBlock> find_suite_blocks(const std::string &src) {
  std::vector<AutotunerBlock> blocks;
  // Match:  autotuner <Name> -> (int passed, int failed)
  std::regex sig_re(R"(\bautotuner\s+([A-Za-z_][A-Za-z0-9_]*)\s*->\s*\(\s*int\s+passed\s*,\s*int\s+failed\s*\))");
  auto it  = std::sregex_iterator(src.begin(), src.end(), sig_re);
  auto end = std::sregex_iterator();
  for (; it != end; ++it) {
    auto &m = *it;
    std::string name = m[1].str();
    // Find the '{' after the signature
    size_t sig_end = m.position() + m.length();
    size_t open = src.find('{', sig_end);
    if (open == std::string::npos) continue;
    size_t close = find_matching_brace(src, open);
    if (close == std::string::npos) continue;
    blocks.push_back({name, open, close});
  }
  return blocks;
}

// Inside a suite body, find all `state test_*` first-state names.
// We deduplicate: test_multi_step and test_multi_step_b → only test_multi_step.
// A "first" test state is one whose name is NOT referenced by any other
// test_ state's transition (-> test_multi_step_b).  Simple heuristic:
// collect all state test_* names, then remove any that appear as a
// transition target from another state test_* state.
static SuiteInfo analyse_suite_body(const std::string &name,
                                    const std::string &body) {
  SuiteInfo info;
  info.name = name;

  // Collect all state test_* names
  std::regex state_re(R"(\bstate\s+(test_[A-Za-z0-9_]+)\s*\()");
  std::vector<std::string> all_test_states;
  {
    auto it = std::sregex_iterator(body.begin(), body.end(), state_re);
    for (; it != std::sregex_iterator(); ++it)
      all_test_states.push_back((*it)[1].str());
  }

  // Find all transition targets that look like test_*
  std::regex trans_re(R"(->\s*(test_[A-Za-z0-9_]+)\s*\()");
  std::vector<std::string> targets;
  {
    auto it = std::sregex_iterator(body.begin(), body.end(), trans_re);
    for (; it != std::sregex_iterator(); ++it)
      targets.push_back((*it)[1].str());
  }

  // Entry test states = those not targeted by another test_ state
  for (auto &s : all_test_states) {
    bool is_continuation = false;
    for (auto &t : targets)
      if (t == s) { is_continuation = true; break; }
    if (!is_continuation) info.test_names.push_back(s);
  }

  // Detect setup / teardown
  info.has_setup    = std::regex_search(body, std::regex(R"(\bstate\s+setup\s*\()"));
  info.has_teardown = std::regex_search(body, std::regex(R"(\bstate\s+teardown\s*\()"));

  return info;
}

// ── Harness code generator ─────────────────────────────────────────────────
// Returns the harness states to be injected BEFORE the closing `}` of the
// autotuner body.

static std::string generate_harness_states(const SuiteInfo &suite) {
  std::ostringstream o;
  o << "\n  // ── Harness (generated by falcon-test — do not edit) ──────────────\n";

  // __init: register tests, print header, enter loop
  o << "  state __init {\n";
  o << "    TestRunner runner = TestRunner.New(\"" << suite.name << "\");\n";
  bool first = true;
  for (auto &n : suite.test_names) {
    if (first) {
      o << "    Error err = runner.Register(\"" << n << "\");\n";
      first = false;
    } else {
      o << "    err = runner.Register(\"" << n << "\");\n";
    }
  }
  o << "    err = runner.PrintHeader();\n";
  o << "    -> __loop(runner);\n";
  o << "  }\n\n";

  // __loop: check HasNext, branch to setup or __begin or __finish
  o << "  state __loop (TestRunner runner) {\n";
  o << "    bool more = runner.HasNext();\n";
  if (suite.has_setup)
    o << "    if (more) { -> setup(runner);}\n";
  else
    o << "    if (more) { -> __begin(runner);}\n";
  o << "    else      { -> __finish(runner);}\n";
  o << "  }\n\n";

  // __begin: BeginCurrent → __dispatch
  o << "  state __begin (TestRunner runner) {\n";
  o << "    TestContext t = runner.BeginCurrent();\n";
  o << "    -> __dispatch(runner, t);\n";
  o << "  }\n\n";

  // __dispatch: string switch over test names
  o << "  state __dispatch (TestRunner runner, TestContext t) {\n";
  o << "    string name = runner.CurrentName();\n";
  for (auto &n : suite.test_names)
    o << "    if (name == \"" << n << "\") { -> " << n << "(runner, t); }\n";
  // Unknown fallback — record failure, skip to __end
  o << "    Error err = t.Fail(\"no body for: \" + name);\n";
  if (suite.has_teardown)
    o << "    -> teardown(runner, t);\n";
  else
    o << "    -> __end(runner, t);\n";
  o << "  }\n\n";

  // __end: EndCurrent → __loop
  o << "  state __end (TestRunner runner, TestContext t) {\n";
  o << "    Error err = runner.EndCurrent(t);\n";
  o << "    -> __loop(runner);\n";
  o << "  }\n\n";

  // __finish: summary, assign outputs, terminal
  o << "  state __finish (TestRunner runner) {\n";
  o << "    Error err = runner.PrintSummary();\n";
  o << "    passed = runner.PassedCount();\n";
  o << "    failed = runner.FailedCount();\n";
  o << "    terminal;\n";
  o << "  }\n";

  return o.str();
}

// ── Transform a source file ────────────────────────────────────────────────
// Finds every suite autotuner block, injects harness states, returns the
// transformed source.  The import lines and all non-suite content are
// preserved verbatim.

static std::string transform_source(const std::string &src,
                                    std::vector<SuiteInfo> &suites_out) {
  auto blocks = find_suite_blocks(src);
  if (blocks.empty()) return src;

  std::string result = src;
  // Process in reverse order so earlier offsets remain valid after injection.
  std::vector<std::pair<size_t, AutotunerBlock>> indexed;
  for (auto &b : blocks) indexed.push_back({b.close_brace, b});
  std::sort(indexed.begin(), indexed.end(),
            [](auto &a, auto &b){ return a.first > b.first; });

  for (auto &[close_pos, block] : indexed) {
    std::string body = result.substr(block.open_brace + 1,
                                     close_pos - block.open_brace - 1);
    SuiteInfo suite = analyse_suite_body(block.name, body);
    if (suite.test_names.empty()) continue;  // not a test suite
    suites_out.push_back(suite);
    std::string harness = generate_harness_states(suite);
    result.insert(close_pos, harness);
  }
  return result;
}

// ── Usage ──────────────────────────────────────────────────────────────────
static void print_usage(const char *argv0) {
  std::cout <<
    "Usage: " << argv0 << " <file.fal> [file2.fal ...] [options]\n"
    "\n"
    "Runs all test suite autotuners found in the given .fal files.\n"
    "A test suite autotuner has the signature:\n"
    "    autotuner MySuite -> (int passed, int failed)\n"
    "\n"
    "User contract:\n"
    "  - Write  start -> __init;  as the entry (required).\n"
    "  - Optionally define  state setup (TestRunner runner)  ending with\n"
    "    -> __begin(runner);\n"
    "  - Optionally define  state teardown (TestRunner runner, TestContext t)\n"
    "    ending with  -> __end(runner, t);\n"
    "  - Define test states named  state test_<name> (TestRunner runner, TestContext t)\n"
    "    ending with  -> teardown(runner, t);  or  -> __end(runner, t);\n"
    "  - All other imports, structs, routines, and autotuners in the file\n"
    "    are loaded as-is and available to your test bodies.\n"
    "\n"
    "Options:\n"
    "  --log-level L   Log level: trace|debug|info|warn|error  (default: warn)\n"
    "  --dump          Print the generated .fal source and exit (debug)\n"
    "  --help          Show this help\n"
    "\n"
    "Exit codes:  0=all passed  1=usage error  2=load error  3=tests failed\n";
}

// ── Main ───────────────────────────────────────────────────────────────────

int main(int argc, char *argv[]) {
  if (argc < 2) { print_usage(argv[0]); return 1; }

  std::vector<std::string> fal_files;
  std::string log_level = "warn";
  bool dump_mode = false;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--help" || arg == "-h") { print_usage(argv[0]); return 0; }
    if (arg == "--dump")                { dump_mode = true; continue; }
    if (arg == "--log-level" && i+1 < argc) { log_level = argv[++i]; continue; }
    if (arg.size() > 4 && arg.substr(arg.size()-4) == ".fal") {
      fal_files.push_back(arg); continue;
    }
    std::cerr << "error: unrecognised argument: " << arg << "\n";
    return 1;
  }

  if (fal_files.empty()) {
    std::cerr << "error: no .fal files specified\n";
    print_usage(argv[0]);
    return 1;
  }

  setenv("LOG_LEVEL", log_level.c_str(), 1);

  int total_failed = 0;

  for (auto &fal_path : fal_files) {
    // ── Read source ──────────────────────────────────────────────────────
    std::string src;
    try { src = read_file(fal_path); }
    catch (const std::exception &e) {
      std::cerr << "error: " << e.what() << "\n"; return 2;
    }

    // ── Transform: inject harness states ────────────────────────────────
    std::vector<SuiteInfo> suites;
    std::string generated = transform_source(src, suites);

    if (suites.empty()) {
      std::cerr << "warning: no test suite autotuners found in " << fal_path << "\n"
                << "         (suites need signature: autotuner Name -> (int passed, int failed))\n";
      continue;
    }

    if (dump_mode) {
      std::cout << "// ── Generated source for: " << fal_path << " ──\n"
                << generated << "\n";
      continue;
    }

    // ── Write generated file next to original (same dir, for import resolution)
    fs::path orig(fal_path);
    fs::path gen_path = orig.parent_path() /
                        ("." + orig.stem().string() + "_faltest_generated.fal");
    try { write_file(gen_path.string(), generated); }
    catch (const std::exception &e) {
      std::cerr << "error: " << e.what() << "\n"; return 2;
    }

    // Print discovery
    std::cout << "falcon-test: " << fal_path << "\n";
    for (auto &s : suites) {
      std::cout << "  suite [" << s.name << "]  "
                << s.test_names.size() << " test(s)";
      if (s.has_setup)    std::cout << "  setup";
      if (s.has_teardown) std::cout << "  teardown";
      std::cout << "\n";
    }
    std::cout << "\n";

    // ── Load generated file — imports resolve from original's directory ──
    falcon::dsl::AutotunerEngine engine;
    if (!engine.load_fal_file(gen_path.string())) {
      std::cerr << "error: generated harness failed to load\n"
                << "       run with --dump to inspect the generated source\n";
      fs::remove(gen_path);
      return 2;
    }

    // ── Run every discovered suite ───────────────────────────────────────
    // Reverse because transform_source processes in reverse insertion order;
    // restore file order.
    std::reverse(suites.begin(), suites.end());

    for (auto &suite : suites) {
      try {
        falcon::typing::ParameterMap inputs;
        auto results = engine.run_autotuner(suite.name, inputs);

        int64_t failed = 0;
        if (results.size() >= 2)
          failed = std::get<int64_t>(results[1]);
        total_failed += static_cast<int>(failed);

      } catch (const std::exception &e) {
        std::cerr << "runtime error in suite [" << suite.name << "]: "
                  << e.what() << "\n";
        fs::remove(gen_path);
        return 3;
      }
    }

    fs::remove(gen_path);
  }

  if (dump_mode) return 0;
  return (total_failed == 0) ? 0 : 3;
}
