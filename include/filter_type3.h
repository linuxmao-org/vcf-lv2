#ifndef FILTER_TYPE3_H
#define FILTER_TYPE3_H

typedef struct {
  float *input;
  float *output;
  float *gain;
  float *freq_ofs;
  float *freq_pitch;
  float *reso_ofs;
  float *dBgain_ofs;
  double rate, buf[4];
} filtType3;


typedef struct {
  float *input;
  float *output;
  float *gain;
  float *freq_ofs;
  float *freq_pitch;
  float *reso_ofs;
  float *dBgain_ofs;
  float *freq_in;
  float *reso_in;
  float *dBgain_in;
  double rate, buf[4];
} filtType3_cv;

#endif
