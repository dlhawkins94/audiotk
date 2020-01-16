#include "source.hpp"

pa_source::pa_source() {
  spec.format = PA_SAMPLE_S16NE;
  spec.channels = 1;
  spec.rate = SAMPLE_RATE;

  pulse = pa_simple_new(NULL, // default server
			"speech_recognition", // app name
			PA_STREAM_RECORD, // stream direction
			NULL, // default audio device
			"speech_recognition source", // stream desc
			&spec, // sample format
			NULL, // default channel map
			NULL, // default buffering attribs
			NULL); // ignore error code
}

pa_source::~pa_source() {
  pa_simple_free(pulse);
}

bus pa_source::apply(bus b) {
  vector<int16_t> samples(FRAME_N, 0);
  int res = pa_simple_read(pulse, samples.data(), 2*FRAME_N, NULL);
  
  frame f(FRAME_N);
  for (int i = 0; i < FRAME_N; i++)
    f[i] = complex<double>(((double) samples[i]) / 32768, 0);
  
  return bus({f});
}

file_source::file_source(string path)
  : path(path), file(path, ios::binary)
{
  char file_header[16];
  int ext_data, sample_rate, byte_rate;
  short sample_type, channels, blk_size, bits_per_sample;
  char data_header[8];

  file.read(file_header, sizeof(file_header));
  file.read((char*) &ext_data, sizeof(ext_data));
  file.read((char*) &sample_rate, sizeof(sample_rate));
  file.read((char*) &byte_rate, sizeof(byte_rate));
  file.read((char*) &sample_type, sizeof(sample_type));
  file.read((char*) &channels, sizeof(channels));
  file.read((char*) &blk_size, sizeof(blk_size));
  file.read((char*) &bits_per_sample, sizeof(bits_per_sample));
  file.read(data_header, sizeof(data_header));

  if (channels > 2) cerr << "channel count must be mono!" << endl;
}

bus file_source::apply(bus b) {
  int16_t buffer[FRAME_N];
  frame f(FRAME_N);
  
  file.read((char*) buffer, FRAME_N * 2);
  if (!file.eof()) {
    for (int n = 0; n < FRAME_N; n++)
      f[n] = (complex<double>) ((double) buffer[n]) / 32768.0;

    return bus({f});
  }
  else return bus({});
}
