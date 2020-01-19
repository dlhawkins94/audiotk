#include <csignal>
#include <iostream>

#include <audiotk/audiotk.hpp>

using namespace std;

sig_atomic_t sgnl = 0;
void handle(int sig) { sgnl = sig; }

int main(int argc, char **argv) {
  signal(SIGINT, handle);
  
  glfwInit();

  pa_source in;
  int N_wt = 8;
  fwt wt(N_wt, 256, "D4");
  multiscope scope1(1024, 768,
		    wt.sizes(),
		    vector<double>(N_wt, -0.5),
		    vector<double>(N_wt, 0.5));
  ifwt iwt(N_wt, 256, "D4");
  pa_sink out;

  stopwatch st;
  while (sgnl != SIGINT) {
    bus b = in.apply({});
    bus b_ = wt.apply(b);
    scope1.apply(b_);
    b = iwt.apply(b_);
    out.apply(b);
  }
  
  glfwTerminate();
  return 0;
}
