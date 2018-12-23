/***************** Bandpass2 II, Formula by Robert Bristow-Johnson ******************/
/*  LADSPA version by Matthias Nagorni
    LV2 port by James W. Morris <james@jwm-art.net>
*/

#include <stdlib.h>
#include <math.h>
#include <lv2.h>

#include "vcf.h"
#include "filter_type2.h"

#define BANDPASS2_URI   "http://jwm-art.net/lv2/vcf/bandpass2";
#define BANDPASS2CV_URI "http://jwm-art.net/lv2/vcf/bandpass2_cv";

static LV2_Descriptor *Bandpass2Descriptor = NULL;
static LV2_Descriptor *Bandpass2CVDescriptor = NULL;

typedef filtType2    Bandpass2;
typedef filtType2_cv Bandpass2CV;

static void cleanupBandpass2(LV2_Handle instance)
{
    free(instance);
}

static void connectPortBandpass2(
    LV2_Handle instance, uint32_t port, void *data)
{
    Bandpass2 *plugin = (Bandpass2 *)instance;
    switch(port){
        case 0: plugin->input = data;       break;
        case 1: plugin->output = data;      break;
        case 2: plugin->gain = data;        break;
        case 3: plugin->freq_ofs = data;    break;
        case 4: plugin->freq_pitch = data;  break;
        case 5: plugin->reso_ofs = data;    break;
    }
}

static LV2_Handle instantiateBandpass2(
    const LV2_Descriptor *descriptor,
    double s_rate,
    const char *path,
    const LV2_Feature * const* features)
{
    Bandpass2* plugin_data = (Bandpass2*)malloc(sizeof(Bandpass2));
    plugin_data->rate = s_rate;
    return (LV2_Handle)plugin_data;
}

static void activateBandpass2(LV2_Handle instance)
{
    Bandpass2 *plugin_data = (Bandpass2 *)instance;
    int l1;
    for (l1 = 0; l1 < 4; l1++)
      plugin_data->buf[l1] = 0;
}

static void runBandpass2(LV2_Handle instance, uint32_t sample_count)
{
    uint32_t l1;
    double f, q, pi2_rate;
    double *buf;
    double iv_sin, iv_cos, iv_alpha, inv_a0, a0, a1, a2, b0, b1, b2;
    Bandpass2 *pluginData = (Bandpass2 *)instance;
    float *input = pluginData->input;
    float *output = pluginData->output;
    float gain = *(pluginData->gain);
    float freq_pitch =
        (*(pluginData->freq_pitch) > 0)
            ? 1.0 + *(pluginData->freq_pitch) / 2.0
        : 1.0 / (1.0 - *(pluginData->freq_pitch) / 2.0);
    pi2_rate = 2.0 * M_PI / pluginData->rate;
    buf = pluginData->buf;
    f = *(pluginData->freq_ofs) * freq_pitch;
    if (f < MIN_FREQ)
        f = MIN_FREQ;
    if (f > MAX_FREQ)
        f = MAX_FREQ;
    q = *(pluginData->reso_ofs);
    if (q < Q_MIN)
        q = Q_MIN;
    if (q > Q_MAX)
        q = Q_MAX;
    iv_sin = sin(pi2_rate * f);
    iv_cos = cos(pi2_rate * f);
    iv_alpha = iv_sin/(Q_SCALE * q);
    b0 = iv_alpha;
    b1 = 0;
    b2 = -iv_alpha;
    a0 = 1.0 + iv_alpha;
    a1 = -2.0 * iv_cos;
    a2 = 1.0 - iv_alpha;
    inv_a0 = 1.0 / a0;
    for (l1 = 0; l1 < sample_count; l1++) {
        output[l1] = inv_a0 * (gain
            * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1];
    }
}

static void initBandpass2()
{
    Bandpass2Descriptor =
        (LV2_Descriptor *) malloc(sizeof(LV2_Descriptor));
    Bandpass2Descriptor->URI =              BANDPASS2_URI;
    Bandpass2Descriptor->activate =         activateBandpass2;
    Bandpass2Descriptor->cleanup =          cleanupBandpass2;
    Bandpass2Descriptor->connect_port =     connectPortBandpass2;
    Bandpass2Descriptor->deactivate =       NULL;
    Bandpass2Descriptor->instantiate =      instantiateBandpass2;
    Bandpass2Descriptor->run =              runBandpass2;
    Bandpass2Descriptor->extension_data =   NULL;
}

static void cleanupBandpass2CV(LV2_Handle instance)
{
    free(instance);
}

static void connectPortBandpass2CV(
    LV2_Handle instance, uint32_t port, void *data)
{
    Bandpass2CV *plugin = (Bandpass2CV *)instance;
    switch(port){
        case 0: plugin->input = data;       break;
        case 1: plugin->output = data;      break;
        case 2: plugin->gain = data;        break;
        case 3: plugin->freq_ofs = data;    break;
        case 4: plugin->freq_pitch = data;  break;
        case 5: plugin->freq_in = data;     break;
        case 6: plugin->reso_ofs = data;    break;
        case 7: plugin->reso_in = data;     break;
    }
}

static LV2_Handle instantiateBandpass2CV(
    const LV2_Descriptor *descriptor,
    double s_rate,
    const char *path,
    const LV2_Feature * const* features)
{
    Bandpass2CV* plugin_data = (Bandpass2CV*)malloc(sizeof(Bandpass2CV));
    plugin_data->rate = s_rate;
    return (LV2_Handle)plugin_data;
}

static void activateBandpass2CV(LV2_Handle instance)
{
    Bandpass2CV *plugin_data = (Bandpass2CV *)instance;
    int l1;
    for (l1 = 0; l1 < 4; l1++)
      plugin_data->buf[l1] = 0;
}

static void runBandpass2CV(LV2_Handle instance, uint32_t sample_count)
{
    uint32_t l1;
    double f0, q0, f, q, pi2_rate;
    double *buf;
    double iv_sin, iv_cos, iv_alpha, inv_a0, a0, a1, a2, b0, b1, b2;
    Bandpass2CV *pluginData = (Bandpass2CV *)instance;
    float *input = pluginData->input;
    float *output = pluginData->output;
    float gain = *(pluginData->gain);
    float freq_ofs = *(pluginData->freq_ofs);
    float freq_pitch =
        (*(pluginData->freq_pitch) > 0)
            ? 1.0 + *(pluginData->freq_pitch) / 2.0
        : 1.0 / (1.0 - *(pluginData->freq_pitch) / 2.0);
    float reso_ofs = *(pluginData->reso_ofs);
    float *freq_in = pluginData->freq_in;
    float *reso_in = pluginData->reso_in;
    pi2_rate = 2.0 * M_PI / pluginData->rate;
    buf = pluginData->buf;
    f0 = freq_ofs;
    q0 = reso_ofs;
    if (!(freq_in || reso_in)) {
        f = f0 * freq_pitch;
        if (f < MIN_FREQ)
            f = MIN_FREQ;
        if (f > MAX_FREQ)
            f = MAX_FREQ;
        q = q0;
        if (q < Q_MIN)
            q = Q_MIN;
        if (q > Q_MAX)
            q = Q_MAX;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        b0 = iv_alpha;
        b1 = 0;
        b2 = -iv_alpha;
        a0 = 1.0 + iv_alpha;
        a1 = -2.0 * iv_cos;
        a2 = 1.0 - iv_alpha;
        inv_a0 = 1.0 / a0;
        for (l1 = 0; l1 < sample_count; l1++) {
            output[l1] = inv_a0 * (gain
                * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                    - a1 * buf[2] - a2 * buf[3]);
            buf[1] = buf[0];
            buf[0] = input[l1];
            buf[3] = buf[2];
            buf[2] = output[l1];
        }
    }
    else {
        if (!reso_in) {
            q = q0;
            if (q < Q_MIN)
                q = Q_MIN;
            if (q > Q_MAX)
                q = Q_MAX;
            for (l1 = 0; l1 < sample_count; l1++) {
                f = (freq_in[l1] > 0)
                    ? (freq_in[l1] * MAX_FREQ
                        + f0 - MIN_FREQ) * freq_pitch
                    : f0 * freq_pitch;
                if (f < MIN_FREQ)
                    f = MIN_FREQ;
                if (f > MAX_FREQ)
                    f = MAX_FREQ;
                iv_sin = sin(pi2_rate * f);
                iv_cos = cos(pi2_rate * f);
                iv_alpha = iv_sin/(Q_SCALE * q);
                b0 = iv_alpha;
                b1 = 0;
                b2 = -iv_alpha;
                a0 = 1.0 + iv_alpha;
                a1 = -2.0 * iv_cos;
                a2 = 1.0 - iv_alpha;
                output[l1] = 1.0 / a0 * (gain
                    * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                        - a1 * buf[2] - a2 * buf[3]);
                buf[1] = buf[0];
                buf[0] = input[l1];
                buf[3] = buf[2];
                buf[2] = output[l1];
            }
        }
        else {
            for (l1 = 0; l1 < sample_count; l1++) {
                f = (freq_in && (freq_in[l1] > 0)) 
                    ? (freq_in[l1] * MAX_FREQ
                        + f0 - MIN_FREQ) * freq_pitch
                    : f0 * freq_pitch;
                if (f < MIN_FREQ)
                    f = MIN_FREQ;
                if (f > MAX_FREQ)
                    f = MAX_FREQ;
                q = q0  + reso_in[l1];
                if (q < Q_MIN)
                    q = Q_MIN;
                if (q > Q_MAX)
                    q = Q_MAX;
                iv_sin = sin(pi2_rate * f);
                iv_cos = cos(pi2_rate * f);
                iv_alpha = iv_sin/(Q_SCALE * q);
                b0 = iv_alpha;
                b1 = 0;
                b2 = -iv_alpha;
                a0 = 1.0 + iv_alpha;
                a1 = -2.0 * iv_cos;
                a2 = 1.0 - iv_alpha;
                inv_a0 = 1.0 / a0;
                output[l1] = inv_a0 * (gain
                    * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                        - a1 * buf[2] - a2 * buf[3]);
                buf[1] = buf[0];
                buf[0] = input[l1];
                buf[3] = buf[2];
                buf[2] = output[l1];
            }
        }
    }
}

static void initBandpass2CV()
{
    Bandpass2CVDescriptor =
        (LV2_Descriptor *) malloc(sizeof(LV2_Descriptor));
    Bandpass2CVDescriptor->URI =              BANDPASS2CV_URI;
    Bandpass2CVDescriptor->activate =         activateBandpass2CV;
    Bandpass2CVDescriptor->cleanup =          cleanupBandpass2CV;
    Bandpass2CVDescriptor->connect_port =     connectPortBandpass2CV;
    Bandpass2CVDescriptor->deactivate =       NULL;
    Bandpass2CVDescriptor->instantiate =      instantiateBandpass2CV;
    Bandpass2CVDescriptor->run =              runBandpass2CV;
    Bandpass2CVDescriptor->extension_data =   NULL;
}

LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
    switch(index){
        case 0:
            if (!Bandpass2Descriptor)
                initBandpass2();
            return Bandpass2Descriptor;
        case 1:
            if (!Bandpass2CVDescriptor)
                initBandpass2CV();
            return Bandpass2CVDescriptor;
    }
    return NULL;
}
