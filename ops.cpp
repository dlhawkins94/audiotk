#include "ops.hpp"

average::average(int K, int M, int N)
  : systm("average", {{N,M}}), K(K), k(0),
    old_avg(MatrixXcd::Zero(M,N)),
    avg(MatrixXcd::Zero(M,N))
{}

bus average::apply(bus b) {
  avg += b[0].samples;
  
  if (++k == K) {
    k = 0;
    old_avg = avg / (double) K;
    avg *= 0.0;
  }
  
  frame f = b[0];
  f.samples = old_avg;
  f.flags = !k;
  return bus({f});
}

bus unitize(int n, int N) {
  frame f(N);
  f[n] += 1.0;
  return bus({f});
}

double frame_power(frame f) {
  double total = 0.0;
  for (int n = 0; n < f.samples.size(); n++)
    total += pow(abs(f[n]), 2);
  return total / (double) f.samples.size();
}

