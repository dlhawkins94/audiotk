#pragma once

#include <ccomplex>
#include <cmath>
#include <complex>
#include <iostream>
#include <string>
#include <vector>

#include <eigen3/Eigen/Dense>

#define SAMPLE_RATE 16000 // Hz
#define FRAME_DURATION 10 // ms
#define FRAME_N FRAME_DURATION * SAMPLE_RATE / 1000 // samples
#define FRAME_N2 FRAME_N / 2

using namespace std;
using namespace Eigen;

/*
 * Passing one dimension yields a vector frame.
 * Passing two dimensions yields a matrix frame.
 * For now, [] operator treats samples like it's a vector.
 */
struct frame {
  MatrixXcd samples;
  int flags;
  
  frame(unsigned int N)
    : samples(VectorXcd::Zero(N)), flags(0) {}
  
  frame(unsigned int M, unsigned int N)
    : samples(MatrixXcd::Zero(M,N)), flags(0) {}

  ~frame() {}
  
  complex<double> operator [] (int n) const { return samples(n); }
  complex<double> & operator [] (int n) { return samples(n); }
};

typedef vector<frame> bus;

class systm {
  string desc;
  vector<vector<int>> dims;

protected:
  systm(string desc, vector<vector<int>> dims) : desc(desc), dims(dims) {}
  
public:
  virtual ~systm() {}
  string get_desc() { return desc; }
  vector<vector<int>> get_dims() { return dims; }
  void add_dims(vector<int> dim) { dims.push_back(dim); }
  virtual bus apply(bus b) = 0;
  frame apply_siso(frame f) { return apply(bus({f}))[0]; }
};

class overlap {
  frame in, out;
  
public:
  overlap();
  virtual frame apply_once(frame f);
  bus apply(bus b);
};

