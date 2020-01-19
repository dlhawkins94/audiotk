#pragma once

#include <fftw3.h>

#include "dsp_core.hpp"

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

/*
 * Obtains cepstral coefficients from the frame.
 */
class cepstrum : public systm {
  mel fb;
  dct tr;

public:
  cepstrum(int K);
  bus apply(bus b);
};

class haar_dwt : public systm {
  int N, K;
  bool cat;

  bus calculate(frame gprev, int n);

public:
  haar_dwt(int N, int K, bool cat);
  bus apply(bus b);
};

class fwt : public systm {
  int N, K, M;
  string fname;
  VectorXd g, h;

public:
  fwt(int N, int K, string fname);
  vector<int> sizes();
  bus apply(bus b);
};

class ifwt : public systm {
  int N, K, M;
  string fname;
  VectorXd g, h;

public:
  ifwt(int N, int K, string fname);
  bus apply(bus b);
};
