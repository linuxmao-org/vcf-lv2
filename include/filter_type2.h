#ifndef FILTER_TYPE2_H
#define FILTER_TYPE2_H

typedef struct {
  float *input;
  float *output;
  float *gain;
  float *freq_ofs;
  float *freq_pitch;
  float *reso_ofs;
  double rate, buf[4];
} filtType2;

typedef struct {
  float *input;
  float *output;
  float *gain;
  float *freq_ofs;
  float *freq_pitch;
  float *reso_ofs;
  float *freq_in;
  float *reso_in; 
  double rate, buf[4];
} filtType2_cv;

#endif
