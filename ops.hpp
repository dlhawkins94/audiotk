#pragma once

#include "dsp_core.hpp"

class average : public systm {
  int K, k;
  MatrixXcd old_avg, avg;
  
public:
  average(int K, int M, int N);
  bus apply(bus b);
};

bus unitize(int n, int N);
double frame_power(frame f);

