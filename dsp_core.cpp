#include "dsp_core.hpp"

overlap::overlap()
  : in(FRAME_N), out(FRAME_N) {}

frame overlap::apply_once(frame f) {
  return f;
}
  
bus overlap::apply(bus b) {
  frame f = b[0];
  
  // Shift in first half of input frame
  for (int n = 0; n < FRAME_N2; n++) {
    in[n] = in[FRAME_N2 + n];
    in[FRAME_N2 + n] = f[n];
  }

  // Apply the systm to this input, add it to output frame
  frame stage_1 = this->apply_once(in);
  for (int n = 0; n < FRAME_N; n++)
    out[n] += stage_1[n] / 2.0;

  // Shift in rest of input frame
  for (int n = 0; n < FRAME_N2; n++) {
    in[n] = in[FRAME_N2 + n];
    in[FRAME_N2 + n] = f[FRAME_N2 + n];
  }

  // Apply the systm to that. Add first half of result to second half
  // of the output. copy the output buffer's state here, that will be
  // returned.
  frame stage_2 = this->apply_once(in);
  for (int n = 0; n < FRAME_N2; n++)
    out[FRAME_N2 + n] +=  stage_2[n] / 2.0;
  frame result = out;

  // Set up new output frame, overlapped with second half of stage 2.
  for (int n = 0; n < FRAME_N2; n++) {
    out[n] = stage_2[FRAME_N2 + n] / 2.0;
    out[FRAME_N2 + n] = 0;
  }

  return bus({result});
}

fft::fft(int K) : systm("fft", {{1, K}}), K(K) {
  in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * K);
  out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * K);
  p = fftw_plan_dft_1d(K, in, out, FFTW_FORWARD, FFTW_MEASURE);
}

fft::~fft() {
  fftw_destroy_plan(p);
  fftw_free(in); fftw_free(out);
}

bus fft::apply(bus b) {
  frame f = b[0];
  memcpy(in, f.samples.data(), K * sizeof(complex<double>));
  fftw_execute(p);
  
  frame F(K);
  memcpy(F.samples.data(), out, K * sizeof(complex<double>));
  return bus({F});
}

ifft::ifft(int K) : systm("ifft", {{1, K}}), K(K) {
  in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * K);
  out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * K);
  p = fftw_plan_dft_1d(K, in, out, FFTW_BACKWARD, FFTW_MEASURE);
}

ifft::~ifft() {
  fftw_destroy_plan(p);
  fftw_free(in); fftw_free(out);
}

bus ifft::apply(bus B) {
  frame F = B[0];
  memcpy(in, F.samples.data(), K * sizeof(complex<double>));
  fftw_execute(p);
  frame f(K);
  memcpy(f.samples.data(), out, K * sizeof(complex<double>));
  
  f.samples /= (double) K;
  return bus({f});
}

dct::dct(int K) : systm("dct", {{1, K}}), K(K), dft(K), cplx(K) {
  complex<double> I(0.0, 1.0);
  for (int k = 0; k < K; k++)
    cplx(k) = 2.0 * exp(-M_PI * I * (double) k / (double) K / 2.0);
}

/*
 * -- x = [a, b, c, d, e, f] -> x^ = [a, c, e, f, d, b]
 * -- X^ = fft{x^}
 * -- X = real{X^ * 2exp(-jk/2N * pi)}
 */
bus dct::apply(bus b) {
  frame f = b[0];
  frame x_(K);
  for (int k; k < K/2; k++) {
    x_[k] = f[2 * k];
    x_[K - 1 - k] = f[2 * k + 1];
  }
  
  frame F = dft.apply(bus({x_}))[0];
  for (int k = 0; k < K; k++) {
    double Freal = real(F[k] * cplx[k]);
    F[k] = complex<double>(Freal, 0.0);
  }
  return bus({F});
}

double melsc(double x) { return 2595 * log10(1 + x/700); }
double melinv(double x) { return 700 * (pow(10, x / 2595) - 1); }

mel::mel(int K) : systm("mel", {{1, 24}}), K(K), dft(K) {
  double fstart = 64;
  double fs = SAMPLE_RATE;
  
  cbin[0] = (int) round(K * fstart / fs);
  for (int i = 1; i < 24; i++) {
    double i_ = i;
    double fc = melinv(melsc(fstart) + i_ * (melsc(fs/2) - melsc(fstart))/24);
    cbin[i] = (int) round(K * fc / fs);
  }
  cbin[24] = (int) round(K / 2);
}

bus mel::apply(bus b) {
  // Need to zero-pad frame before taking fft
  // If frame is bigger than K there will be data loss.
  frame f = b[0];
  frame f_(K);
  for (int k = 0; k < f.samples.size(); k++)
    f_[k] = f[k];
  
  frame F = dft.apply(bus({f_}))[0];
  
  frame out(24); // mel coefficients
  for (int i = 1; i < 24; i++) {
    for (int k = cbin[i-1]; k <= cbin[i]; k++) {
      double k_ = k;
      out[i] += abs(F[k]) * (((double) (k_ - cbin[i-1] + 1))
			     / ((double) (cbin[i] - cbin[i-1] + 1)));
    }
    
    for (int k = cbin[i] + 1; k <= cbin[i+1]; k++) {
      double k_ = k;
      out[i] += abs(F[k]) * (1 - ((double) (k_ - cbin[i]))
			     / ((double) (cbin[i+1] - cbin[i] + 1)));
    }
    
    out[i] = log(out[i]);
    //if (real(out[i]) < -50.0) out[i] = -50.0;
  }
  //out.samples.normalize();
  out.flags = f.flags;
  return bus({out});
}

cepstrum::cepstrum(int K)
  : systm("cepstrum", {{12, 1}}), fb(K), tr(24) {}

bus cepstrum::apply(bus b) {
  complex<double> I(0.0, 1.0);

  frame f = b[0];
  frame F = fb.apply_siso(f);
  frame C = tr.apply_siso(F);
  
  VectorXcd x = C.samples.cwiseAbs().cast<complex<double>>();
  C.samples = x.segment(1,12);
  C.samples.normalize();
  C.flags = f.flags;
  return bus({C});
}

haar_dwt::haar_dwt(int N)
  : systm("haar_dwt", {}),
    N(N)
{}

bus haar_dwt::apply(bus b) {
}
