#include "dsp_core.hpp"
#include "supersystem.hpp"
#include "scope.hpp"
#include "sink.hpp"
#include "source.hpp"

int main(int argc, char **argv) {
  glfwInit();
  
  supersystem dash;
  
  dash.append_sys("in", new pa_source());
  dash.append_sys("cepstrum", new cepstrum(256));
  dash.append_sys("scope", new waterfall(24, 100, 480, 480, 0.0, 1.0));
  dash.append_sys("out", new pa_sink());

  dash.link_node("cepstrum", "in", 0);
  dash.link_node("scope", "cepstrum", 0);
  dash.link_node("out", "in", 0);

  dash.append_sink("scope");
  dash.append_sink("out");

  for (int i = 0; i > -1; i++)
    dash.apply({});

  glfwTerminate();
  return 0;
}
