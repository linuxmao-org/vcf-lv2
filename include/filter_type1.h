#ifndef FILTER_TYPE1_H
#define FILTER_TYPE1_H

typedef struct {
  float *input;
  float *output;
  float *gain;
  float *freq_ofs;
  float *freq_pitch;
  float *reso_ofs;
  double rate, buf[2];
} filtType1;

typedef struct {
  float *input;
  float *output;
  float *gain;
  float *freq_ofs;
  float *freq_pitch;
  float *reso_ofs;
  float *freq_in;
  float *reso_in;
  double rate, buf[2];
} filtType1_cv;

#endif
