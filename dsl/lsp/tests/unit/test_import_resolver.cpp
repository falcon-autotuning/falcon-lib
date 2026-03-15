#include "falcon-lsp/FalconDocument.hpp"
#include "falcon-lsp/ImportResolver.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

using namespace falcon::lsp;

// Helper: write a string to a temp .fal file and return its path.
static std::string write_temp_fal(const std::string &contents,
                                  const std::string &name = "tmp") {
  namespace fs = std::filesystem;
  auto dir = fs::temp_directory_path();
  auto path = (dir / (name + ".fal")).string();
  std::ofstream f(path);
  f << contents;
  return path;
}

// ── resolve_document populates symbols from imported file ─────────────────

TEST(ImportResolver, ResolvesFileOnDisk) {
  // Create imported file
  const std::string imported_src = R"(
autotuner Adder (int a, int b) -> (int result) {
  result = a + b;
  start -> done;
  state done { terminal; }
}
)";
  std::string imported_path = write_temp_fal(imported_src, "Adder");

  // Create importer
  const std::string src = R"(
import (")" + std::filesystem::path(imported_path).filename().string() +
                          R"(")
autotuner Main () -> () {
  start -> done;
  state done { terminal; }
}
)";
  std::string main_path = write_temp_fal(src, "Main");

  FalconDocumentParser parser;
  FalconDocument doc = parser.parse("file://" + main_path, src);

  ImportResolver resolver;
  resolver.resolve_document("file://" + main_path, doc, parser);

  bool found_adder = false;
  for (const auto &sym : doc.symbols) {
    if (sym.name == "Adder" && sym.kind == "autotuner" && sym.from_import) {
      found_adder = true;
    }
  }
  EXPECT_TRUE(found_adder) << "Adder autotuner should be imported";

  // No unresolved-import warnings
  auto diags = resolver.check_imports(doc);
  bool any_import_warn = false;
  for (const auto &d : diags) {
    if (d.message.find("Import not yet loaded") != std::string::npos)
      any_import_warn = true;
  }
  EXPECT_FALSE(any_import_warn);

  // Cleanup
  std::remove(imported_path.c_str());
  std::remove(main_path.c_str());
}

// ── Cycle detection: A imports B imports A ────────────────────────────────

TEST(ImportResolver, CycleDetection) {
  namespace fs = std::filesystem;
  auto dir = fs::temp_directory_path();
  std::string path_a = (dir / "CycleA.fal").string();
  std::string path_b = (dir / "CycleB.fal").string();

  {
    std::ofstream fa(path_a);
    fa << "import (\"CycleB.fal\")\n"
       << "autotuner A () -> () { start -> s; state s { terminal; } }\n";
  }
  {
    std::ofstream fb(path_b);
    fb << "import (\"CycleA.fal\")\n"
       << "autotuner B () -> () { start -> s; state s { terminal; } }\n";
  }

  FalconDocumentParser parser;
  std::ifstream fa_in(path_a);
  std::string src_a((std::istreambuf_iterator<char>(fa_in)),
                    std::istreambuf_iterator<char>());

  FalconDocument doc = parser.parse("file://" + path_a, src_a);

  ImportResolver resolver;
  // Must not hang / crash
  EXPECT_NO_THROW(resolver.resolve_document("file://" + path_a, doc, parser));

  std::remove(path_a.c_str());
  std::remove(path_b.c_str());
}

// ── Missing file keeps warning ─────────────────────────────────────────────

TEST(ImportResolver, MissingFileStillWarns) {
  FalconDocument doc;
  doc.text = "import (\"./DoesNotExist.fal\")\n";
  doc.import_paths = {"./DoesNotExist.fal"};

  FalconDocumentParser parser;
  ImportResolver resolver;
  resolver.resolve_document("file:///some/dir/main.fal", doc, parser);

  auto diags = resolver.check_imports(doc);
  bool found_warn = false;
  for (const auto &d : diags) {
    if (d.message.find("DoesNotExist") != std::string::npos)
      found_warn = true;
  }
  EXPECT_TRUE(found_warn);
}
