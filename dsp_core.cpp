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
