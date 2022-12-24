#ifndef WHISPERHELPER_H
#define WHISPERHELPER_H
#include <cmath>
#include <cstdio>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <whisper/whisper.h>

std::string to_timestamp(int64_t t, bool comma = false) {
  int64_t msec = t * 10;
  int64_t hr = msec / (1000 * 60 * 60);
  msec = msec - hr * (1000 * 60 * 60);
  int64_t min = msec / (1000 * 60);
  msec = msec - min * (1000 * 60);
  int64_t sec = msec / 1000;
  msec = msec - sec * 1000;

  char buf[32];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d%s%03d", (int)hr, (int)min,
           (int)sec, comma ? "," : ".", (int)msec);

  return std::string(buf);
}
int timestamp_to_sample(int64_t t, int n_samples) {
  return std::max(
      0, std::min((int)n_samples - 1, (int)((t * WHISPER_SAMPLE_RATE) / 100)));
}
void replace_all(std::string &s, const std::string &search,
                 const std::string &replace) {
  for (size_t pos = 0;; pos += replace.length()) {
    pos = s.find(search, pos);
    if (pos == std::string::npos)
      break;
    s.erase(pos, search.length());
    s.insert(pos, replace);
  }
}
std::string output_txt(struct whisper_context *ctx) {
  std::stringstream fout;

  const int n_segments = whisper_full_n_segments(ctx);
  for (int i = 0; i < n_segments; ++i) {
    const char *text = whisper_full_get_segment_text(ctx, i);
    fout << text;
  }

  return fout.str();
}
std::string output_vtt(struct whisper_context *ctx) {
  std::stringstream fout;
  fout << "WEBVTT\n\n";

  const int n_segments = whisper_full_n_segments(ctx);
  for (int i = 0; i < n_segments; ++i) {
    const char *text = whisper_full_get_segment_text(ctx, i);
    const int64_t t0 = whisper_full_get_segment_t0(ctx, i);
    const int64_t t1 = whisper_full_get_segment_t1(ctx, i);

    fout << to_timestamp(t0) << " --> " << to_timestamp(t1) << "\n";
    fout << text << "\n\n";
  }

  return fout.str();
}
std::string output_srt(struct whisper_context *ctx) {
  std::stringstream fout;

  const int n_segments = whisper_full_n_segments(ctx);
  for (int i = 0; i < n_segments; ++i) {
    const char *text = whisper_full_get_segment_text(ctx, i);
    const int64_t t0 = whisper_full_get_segment_t0(ctx, i);
    const int64_t t1 = whisper_full_get_segment_t1(ctx, i);

    fout << i + 1 << "\n";
    fout << to_timestamp(t0, true) << " --> " << to_timestamp(t1, true) << "\n";
    fout << text << "\n\n";
  }

  return fout.str();
}
#endif // WHISPERHELPER_H
