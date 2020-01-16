#pragma once

#include <videotk/graphics.hpp>
#include "dsp_core.hpp"
#include "sink.hpp"

using namespace std;

// Displays frames in a waterfall pattern
class waterfall : public sink {
  window win;
  int N, M;
  double min, max;
  point_cloud_2d pc;

public:
  waterfall(int N, int M, int w, int h, double min, double max);
  bus apply(bus b);
};



