#include "transform.hpp"

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

haar_dwt::haar_dwt(int N, int K, bool cat)
  : systm("haar_dwt", {}),
    N(N), K(K), cat(cat)
{}

// g refers to scaling coefficients (LPF output)
// h refers to wavelet coefficients (HPF output)
bus haar_dwt::calculate(frame gprev, int n) {
  int Kprev = gprev.samples.size();
  int Kn = Kprev/2;
  
  if (n == N) return bus({gprev});
  else {
    frame g(Kn), h(Kn);
    for (int k = 0; k < Kn; k++) {
      g[k] = gprev[2*k] + gprev[2*k + 1];
      h[k] = gprev[2*k] - gprev[2*k + 1];      
    }

    bus output = calculate(g, n + 1);
    output.push_back(h);
    return output;
  }
}

bus haar_dwt::apply(bus b) {
  frame f(K);
  for (int k = 0; k < b[0].samples.size(); k++)
    f[k] = b[0][k];

  bus B = calculate(f, 0);
  if (cat) {
    frame F(K);
    int k = 0;
    for (auto &Fn : B) {
      for (int kn = 0; kn < Fn.samples.size(); kn++)
	F[k++] = Fn[kn];
    }
    return bus({F});
  }
  else return B;
}

void gen_wt_filters(string fname, VectorXd &g, VectorXd &h) {
  if (fname == "haar" || fname == "D2")
    g = VectorXd::Ones(2);
  else if (fname == "D4") {
    g = VectorXd::Zero(4);
    g << 0.6830127, 1.1830127, 0.3169873, -0.1830127;
  }

  h = g.reverse();
  for (int m = 0; m < h.size(); m += 2)
    h(m) *= -1.0;
}

fwt::fwt(int N, int K, string fname)
  : systm("fwt", {}),
    N(N), K(K), fname(fname)
{
  gen_wt_filters(fname, g, h);
  M = g.size();
}

vector<int> fwt::sizes() {
  vector<int> Ks({K/2});
  for (int n = 1; n < N; n++)
    Ks.push_back(Ks.back() / 2);
  Ks.push_back(Ks.back());
  return Ks;
}

bus fwt::apply(bus b) {
  bus B;
  frame s_prev(K);
  for (int k = 0; k < b[0].samples.size(); k++)
    s_prev[k] = b[0][k];
  
  for (int n = 0; n < N; n++) {
    int Kn = s_prev.samples.size();
    frame s(Kn/2), d(Kn/2);

    for (int k = 0; k < Kn/2; k++) {
      for (int m = 0; m < M; m++) {
	int k_nm = 2*k + m - M/2;
	if (k_nm >= 0 && k_nm < Kn) {
	  s[k] += 0.5 * g(m) * s_prev[k_nm];
	  d[k] += 0.5 * h(m) * s_prev[k_nm];
	}
      }
    }

    B.push_back(d);
    s_prev = s;
  }
  
  B.push_back(s_prev);
  return B;
}

ifwt::ifwt(int N, int K, string fname)
  : systm("ifwt", {}),
    N(N), K(K), fname(fname)
{
  gen_wt_filters(fname, g, h);
  M = g.size();
}

bus ifwt::apply(bus b) {
  bus::iterator it = b.end();
  frame s = *(--it);
  
  while (it != b.begin()) {
    int K_prev = 2 * s.samples.size();
    frame d = *(--it);

    frame d_up(K_prev), s_up(K_prev);
    for (int k = 0; k < K_prev/2; k++) {
      d_up[2*k] = d[k];
      s_up[2*k] = s[k];
    }

    s = frame(K_prev);
    for (int k = 0; k < K_prev; k++) {
      for (int m = 0; m < M; m++) {
	int k_nm = k - m + M/2;
	if (k_nm >= 0 && k_nm < K_prev) {
	  s[k] += g(m) * s_up[k_nm];
	  s[k] += h(m) * d_up[k_nm];
	}
      }
    }
  }

  frame f(FRAME_N);
  for (int k = 0; k < FRAME_N; k++)
    f[k] = s[k];

  return bus({f});
}
