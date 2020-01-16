#pragma once

#include <ccomplex>
#include <cmath>
#include <complex>
#include <cstring>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <eigen3/Eigen/Dense>
#include <fftw3.h>

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

class fft : public systm {
  int K;
  fftw_complex *in, *out;
  fftw_plan p;

public:
  fft(int K);
  ~fft();
  bus apply(bus b);
};

class ifft : public systm {
  int K;
  fftw_complex *in, *out;
  fftw_plan p;

public:
  ifft(int K);
  ~ifft();
  bus apply(bus B);
};

/*
 * Type II (Makhoul, N, no padding)
 */
class dct : public systm {
  int K;
  VectorXcd cplx;
  fft dft;
  
public:
  dct(int K);
  bus apply(bus b);
};

/*
 * Obtains mel coefficients from the frame.
 * Takes fft of frame, then applies mel filterbank to it.
 */ 
class mel : public systm {
  int K; // fft width. If its power of 2 it will be bigger than frame
  fft dft;
  int cbin[25]; // fft bin indices where each filter channel is centered

public:
  mel(int K);
  bus apply(bus b);
};

class cepstrum : public systm {
  mel fb;
  dct tr;

public:
  cepstrum(int K);
  bus apply(bus b);
};

class haar_dwt : public systm {
  int N;

public:
  haar_dwt(int N);
  bus apply(bus b);
};
