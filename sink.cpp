#include "sink.hpp"

pa_sink::pa_sink() {
  spec.format = PA_SAMPLE_S16NE;
  spec.channels = 1;
  spec.rate = SAMPLE_RATE;

  pulse = pa_simple_new(NULL, // default server
			"speech_recognition", // app name
			PA_STREAM_PLAYBACK, // stream direction
			NULL, // default audio device
			"speech_recognition sink", // stream desc
			&spec, // sample format
			NULL, // default channel map
			NULL, // default buffering attribs
			NULL); // ignore error code
}

pa_sink::~pa_sink() {
  pa_simple_free(pulse);
}

bus pa_sink::apply(bus b) {
  vector<int16_t> samples(FRAME_N, 0);
  for (int i = 0; i < FRAME_N; i++)
    samples[i] = (int16_t) (real(b[0][i]) * 32768);
	 
  int res = pa_simple_write(pulse, samples.data(), 2*FRAME_N, NULL);
  return bus({});
}

void file_sink::write_data(int data, unsigned int size) {
  for (; size; size--, data >>= 8)
    file.put(static_cast<char> (data & 0xff));
}

file_sink::file_sink(string path)
  : file(path, ios::binary)
{
  // Write file headers
  file << "RIFF----WAVEfmt "; // ---- will hold chunk size
  write_data(16, 4); // no extension data
  write_data(1, 2); // integer samples
  write_data(1, 2); // channel count (mono)
  write_data(SAMPLE_RATE, 4); // sample rate
  write_data(SAMPLE_RATE * 1 * 16 / 8, 4); // byte rate
  write_data(2, 2); // data block size (channels * bytes/sample)
  write_data(16, 2); // bits per sample

  // Write data chunk header
  data_chunk_pos = file.tellp();
  file << "data----"; // ---- will hold chunk size
}

file_sink::~file_sink() {
  size_t file_length = file.tellp();
  file.seekp(data_chunk_pos + 4);
  write_data(file_length - data_chunk_pos + 8, 4);
  file.seekp(4);
  write_data(file_length - 8, 4);
}

bus file_sink::apply(bus b) {
  for (int n = 0; n < b[0].samples.size(); n++) {
    int16_t sample = 32768 * real(b[0][n]);
    write_data(sample, 2);
  }
  return bus({});
}

