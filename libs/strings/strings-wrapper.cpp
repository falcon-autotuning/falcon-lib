#include <algorithm>
#include <falcon-typing/FFIHelpers.hpp>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

// ── internal helpers
// ──────────────────────────────────────────────────────────

static std::string unescape_pattern(const std::string &pat) {
  std::string out;
  for (size_t i = 0; i < pat.size(); ++i) {
    if (pat[i] == '\\' && i + 1 < pat.size()) {
      if (pat[i + 1] == '\\') {
        out += '\\';
        ++i;
      } else
        out += pat[i];
    } else
      out += pat[i];
  }
  return out;
}
static void pack_nil(FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// ── Regex match object
// ────────────────────────────────────────────────────────

struct RegexMatch {
  bool matched = false;
  std::string full;                // full match (group 0)
  std::vector<std::string> groups; // capture groups 1..N
};
using RegexMatchSP = std::shared_ptr<RegexMatch>;

static void pack_match(RegexMatchSP p, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "RegexMatch";
  out[0].value.opaque.ptr = new RegexMatchSP(std::move(p));
  out[0].value.opaque.deleter = [](void *q) {
    delete static_cast<RegexMatchSP *>(q);
  };
  *oc = 1;
}

// ── extern "C" ───────────────────────────────────────────────────────────────

extern "C" {

// ── String predicate / search routines
// ─────────────────────────────────────��─

// Contains(string s, string sub) -> (bool found)
void Contains(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
              int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  auto sub = std::get<std::string>(pm.at("sub"));
  pack_results(FunctionResult{s.find(sub) != std::string::npos}, out, 16, oc);
}

// StartsWith(string s, string prefix) -> (bool found)
void StartsWith(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
                int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  auto prefix = std::get<std::string>(pm.at("prefix"));
  bool result =
      s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
  pack_results(FunctionResult{result}, out, 16, oc);
}

// EndsWith(string s, string suffix) -> (bool found)
void EndsWith(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
              int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  auto suffix = std::get<std::string>(pm.at("suffix"));
  bool result = s.size() >= suffix.size() &&
                s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
  pack_results(FunctionResult{result}, out, 16, oc);
}

// Length(string s) -> (int len)
void Length(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
            int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  pack_results(FunctionResult{(int64_t)s.size()}, out, 16, oc);
}

// IsEmpty(string s) -> (bool empty)
void IsEmpty(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
             int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  pack_results(FunctionResult{s.empty()}, out, 16, oc);
}

// ── Transformation routines
// ───────────────────────────────────────────────────

// ToUpper(string s) -> (string result)
void ToUpper(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
             int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
  pack_results(FunctionResult{s}, out, 16, oc);
}

// ToLower(string s) -> (string result)
void ToLower(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
             int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  pack_results(FunctionResult{s}, out, 16, oc);
}

// TrimSpace(string s) -> (string result)
void TrimSpace(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
               int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  // Only trim spaces and tabs, not newlines
  size_t start = s.find_first_not_of(" \t\r\f\v");
  if (start == std::string::npos) {
    pack_results(FunctionResult{std::string("")}, out, 16, oc);
    return;
  }
  size_t end = s.find_last_not_of(" \t\r\f\v");
  std::string trimmed = s.substr(start, end - start + 1);
  pack_results(FunctionResult{trimmed}, out, 16, oc);
}

// Replace(string s, string from, string to) -> (string result)
// Replaces the FIRST occurrence of `from` with `to`.
void Replace(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
             int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  auto from = std::get<std::string>(pm.at("from"));
  auto to = std::get<std::string>(pm.at("to"));
  size_t pos = s.find(from);
  if (pos != std::string::npos)
    s.replace(pos, from.size(), to);
  pack_results(FunctionResult{s}, out, 16, oc);
}

// ReplaceAll(string s, string from, string to) -> (string result)
void ReplaceAll(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
                int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  auto from = std::get<std::string>(pm.at("from"));
  auto to = std::get<std::string>(pm.at("to"));
  if (!from.empty()) {
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
      s.replace(pos, from.size(), to);
      pos += to.size();
    }
  }
  pack_results(FunctionResult{s}, out, 16, oc);
}

// Concat(string a, string b) -> (string result)
void Concat(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
            int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto a = std::get<std::string>(pm.at("a"));
  auto b = std::get<std::string>(pm.at("b"));
  pack_results(FunctionResult{a + b}, out, 16, oc);
}

// Substring(string s, int start, int length) -> (string result)
void Substring(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
               int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  int64_t start = std::get<int64_t>(pm.at("begin"));
  int64_t len = std::get<int64_t>(pm.at("length"));
  if (start < 0)
    start = 0;
  if (start >= (int64_t)s.size()) {
    pack_results(FunctionResult{std::string("")}, out, 16, oc);
    return;
  }
  pack_results(FunctionResult{s.substr(static_cast<size_t>(start),
                                       static_cast<size_t>(len))},
               out, 16, oc);
}

// IndexOf(string s, string sub) -> (int index)
// Returns -1 if not found.
void IndexOf(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
             int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  auto sub = std::get<std::string>(pm.at("sub"));
  size_t pos = s.find(sub);
  int64_t result = (pos == std::string::npos) ? -1 : (int64_t)pos;
  pack_results(FunctionResult{result}, out, 16, oc);
}

// StripAnsi(string s) -> (string result)
// Remove ANSI/VT100 escape sequences (e.g. colour codes from the test runner).
void StripAnsi(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
               int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  static const std::regex ansi_re(R"(\\033\[[0-9;?]*[ -/]*[@-~])");
  std::string result = std::regex_replace(s, ansi_re, "");
  result.erase(std::remove(result.begin(), result.end(), '\\'), result.end());
  pack_results(FunctionResult{result}, out, 16, oc);
}

// ── Conversion routines
// ───────────────────────────────────────────────────────

// FromInt(int i) -> (string result)
void FromInt(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
             int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto i = std::get<int64_t>(pm.at("i"));
  pack_results(FunctionResult{std::to_string(i)}, out, 16, oc);
}

// FromDouble(float f) -> (string result)
void FromDouble(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
                int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto f = std::get<double>(pm.at("f"));
  pack_results(FunctionResult{std::to_string(f)}, out, 16, oc);
}

// FromBool(bool b) -> (string result)
void FromBool(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
              int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto b = std::get<bool>(pm.at("b"));
  pack_results(FunctionResult{std::string(b ? "true" : "false")}, out, 16, oc);
}

// ToInt(string s) -> (int result)
void ToInt(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
           int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  try {
    pack_results(FunctionResult{static_cast<int64_t>(std::stoll(s))}, out, 16,
                 oc);
  } catch (...) {
    pack_results(
        FunctionResult{nullptr, ErrorObject{"Cannot parse int: " + s, false}},
        out, 16, oc);
  }
}

// ToDouble(string s) -> (float result)
void ToDouble(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
              int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  try {
    pack_results(FunctionResult{std::stod(s)}, out, 16, oc);
  } catch (...) {
    pack_results(
        FunctionResult{nullptr,
                       ErrorObject{"Cannot parse double: " + s, false}},
        out, 16, oc);
  }
}

// ToBool(string s) -> (bool result)
void ToBool(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
            int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  if (s == "true" || s == "1") {
    pack_results(FunctionResult{true}, out, 16, oc);
  } else if (s == "false" || s == "0") {
    pack_results(FunctionResult{false}, out, 16, oc);
  } else {
    pack_results(
        FunctionResult{nullptr, ErrorObject{"Cannot parse bool: " + s, false}},
        out, 16, oc);
  }
}

// ── Regex routines
// ────────────────────────────────────────────────────────────

// RegexMatch struct — New / Matched / Group

// RegexMatch.New(string pattern, string text) -> (RegexMatch m)
// Runs a full / partial search match and returns a match object.
void STRUCTRegexMatchNew(const FalconParamEntry *p, int32_t pc,
                         FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto pattern = std::get<std::string>(pm.at("pattern"));
  auto text = std::get<std::string>(pm.at("text"));

  auto m = std::make_shared<RegexMatch>();
  try {
    std::regex re(unescape_pattern(pattern));
    std::smatch sm;
    if (std::regex_search(text, sm, re)) {
      m->matched = true;
      m->full = sm[0].str();
      for (size_t i = 1; i < sm.size(); ++i)
        m->groups.push_back(sm[i].str());
    }
  } catch (const std::regex_error &) {
    m->matched = false;
  }
  pack_match(std::move(m), out, oc);
}

// RegexMatch.Matched() -> (bool matched)
void STRUCTRegexMatchMatched(const FalconParamEntry *p, int32_t pc,
                             FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{get_opaque<RegexMatch>(p, pc, "this")->matched},
               out, 16, oc);
}

// RegexMatch.Full() -> (string full_match)
void STRUCTRegexMatchFull(const FalconParamEntry *p, int32_t pc,
                          FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{get_opaque<RegexMatch>(p, pc, "this")->full}, out,
               16, oc);
}

// RegexMatch.Group(int n) -> (string group)
// Returns capture group n (1-based).  Returns "" if n is out of range.
void STRUCTRegexMatchGroup(const FalconParamEntry *p, int32_t pc,
                           FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  int64_t n = std::get<int64_t>(pm.at("n"));
  auto m = get_opaque<RegexMatch>(p, pc, "this");
  std::string result;
  if (n >= 1 && (size_t)n <= m->groups.size())
    result = m->groups[static_cast<size_t>(n - 1)];
  pack_results(FunctionResult{result}, out, 16, oc);
}

// RegexMatch.GroupCount() -> (int count)
void STRUCTRegexMatchGroupCount(const FalconParamEntry *p, int32_t pc,
                                FalconResultSlot *out, int32_t *oc) {
  int64_t n = (int64_t)get_opaque<RegexMatch>(p, pc, "this")->groups.size();
  pack_results(FunctionResult{n}, out, 16, oc);
}

// RegexTest(string pattern, string text) -> (bool matched)
// Quick predicate — no object allocation.
void RegexTest(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
               int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto pattern = std::get<std::string>(pm.at("pattern"));
  auto text = std::get<std::string>(pm.at("text"));
  bool result = false;
  try {
    result = std::regex_search(text, std::regex(unescape_pattern(pattern)));
  } catch (const std::regex_error &) {
  }
  pack_results(FunctionResult{result}, out, 16, oc);
}

// RegexReplace(string s, string pattern, string replacement) -> (string result)
// Replaces all non-overlapping matches of `pattern` in `s`.
void RegexReplace(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
                  int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto s = std::get<std::string>(pm.at("s"));
  auto pattern = std::get<std::string>(pm.at("pattern"));
  auto replacement = std::get<std::string>(pm.at("replacement"));
  std::string result = s;
  try {
    result = std::regex_replace(s, std::regex(unescape_pattern(pattern)),
                                replacement);
  } catch (const std::regex_error &) {
  }
  pack_results(FunctionResult{result}, out, 16, oc);
}

} // extern "C"
