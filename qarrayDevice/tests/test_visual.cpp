#include "qarrayDevice/Device.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <plplot/plplot.h>
#include <vector>

namespace {

std::string write_temp_config_visual(const std::string &yaml_content) {
  auto tmp = std::filesystem::temp_directory_path() /
             "falcon_qarray_visual_test_config.yaml";
  std::ofstream f(tmp);
  f << yaml_content;
  return tmp.string();
}

constexpr const char *VISUAL_CONFIG = R"yaml(
n_dots: 2
gate_names:
  - P1
  - P2
  - B1
  - S1
sensor_config:
  S1: {}
capacitances:
  Cgd:
    - [1.0, 0.1, 0.1, 0.0]
    - [0.1, 1.0, 0.1, 0.0]
  Cgs:
    - [0.05, 0.05, 0.05, 1.0]
)yaml";

void plot_stability_diagram(const std::string &filename,
                            const std::vector<double> &data, int resolution,
                            double xmin, double xmax, double ymin, double ymax,
                            const std::string &xlabel,
                            const std::string &ylabel) {
  if (data.empty())
    return;

  std::vector<std::vector<PLFLT>> z2d(resolution,
                                      std::vector<PLFLT>(resolution));
  for (int i = 0; i < resolution; ++i)
    for (int j = 0; j < resolution; ++j)
      z2d[j][i] = static_cast<PLFLT>(data[i * resolution + j]);

  std::vector<PLFLT *> zptrs(resolution);
  for (int i = 0; i < resolution; ++i)
    zptrs[i] = z2d[i].data();

  plsetopt("bgcolor", "white");
  plsdev("pngcairo");
  plsfnam(filename.c_str());
  plinit();
  plenv(xmin, xmax, ymin, ymax, 0, 0);
  pllab(xlabel.c_str(), ylabel.c_str(), "Charge Stability Diagram");

  // Find min/max for scaling
  double zmin = data[0], zmax = data[0];
  for (double v : data) {
    if (v < zmin)
      zmin = v;
    if (v > zmax)
      zmax = v;
  }
  if (zmin == zmax)
    zmax += 1.0;

  plimage(zptrs.data(), resolution, resolution, xmin, xmax, ymin, ymax, zmin,
          zmax, xmin, xmax, ymin, ymax);
  plend();
}

void plot_1d(const std::string &filename, const std::vector<double> &x,
             const std::vector<double> &y, const std::string &xlabel,
             const std::string &ylabel, const std::string &title) {
  if (x.empty() || y.empty())
    return;

  plsetopt("bgcolor", "white");
  plsdev("pngcairo");
  plsfnam(filename.c_str());
  plinit();

  double xmin = x[0], xmax = x[0], ymin = y[0], ymax = y[0];
  for (double val : x) {
    if (val < xmin)
      xmin = val;
    if (val > xmax)
      xmax = val;
  }
  for (double val : y) {
    if (val < ymin)
      ymin = val;
    if (val > ymax)
      ymax = val;
  }
  if (ymin == ymax) {
    ymin -= 0.5;
    ymax += 0.5;
  }
  if (xmin == xmax) {
    xmin -= 0.5;
    xmax += 0.5;
  }

  plenv(xmin, xmax, ymin, ymax, 0, 0);
  pllab(xlabel.c_str(), ylabel.c_str(), title.c_str());
  plline(x.size(), x.data(), y.data());
  plend();
}
} // namespace

TEST(DeviceVisual, ChargeStabilityDiagram) {
  auto config_path = write_temp_config_visual(VISUAL_CONFIG);
  falcon::qarray::Device dev(config_path);

  int res = 100;
  double scale = 2.0;
  auto result =
      dev.scan_2d("P1", "P2", {-2 * scale, scale}, {-scale, scale}, res);

  std::vector<double> data;
  if (result.has_sensor && !result.differentiated_signal.empty()) {
    data = result.differentiated_signal;
  } else {
    data.assign(res * res, 0.0);
  }

  std::filesystem::create_directories("plots");
  std::string out_path = "plots/stability_diagram.png";
  plot_stability_diagram(out_path, data, res, -scale, scale, -scale, scale,
                         "P1 (V)", "P2 (V)");
}

TEST(DeviceVisual, Scan1D) {
  auto config_path = write_temp_config_visual(VISUAL_CONFIG);
  falcon::qarray::Device dev(config_path);

  int res = 200;
  auto result = dev.scan_1d("P1", {-1.5, 1.5}, res);

  std::vector<double> x(res);
  for (int i = 0; i < res; ++i) {
    x[i] = -1.5 + (3.0 * i) / (res - 1);
  }

  std::vector<double> y;
  if (result.has_sensor) {
    y = result.sensor_output;
  } else {
    y.assign(res, 0.0);
  }

  std::filesystem::create_directories("plots");
  std::string out_path = "plots/scan_1d.png";
  plot_1d(out_path, x, y, "P1 (V)", "Sensor Output", "1D Gate Scan");
}
