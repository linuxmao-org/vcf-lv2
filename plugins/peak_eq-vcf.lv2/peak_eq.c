/******************* PeakEQ, Formula by Robert Bristow-Johnson ********************/
/*  LADSPA version by Matthias Nagorni
    LV2 port by James W. Morris <james@jwm-art.net>
*/

#include <stdlib.h>
#include <math.h>
#include <lv2.h>

#include "vcf.h"
#include "filter_type3.h"

#define PEAKEQ_URI   "http://jwm-art.net/lv2/vcf/peak_eq";
#define PEAKEQCV_URI "http://jwm-art.net/lv2/vcf/peak_eq_cv";

static LV2_Descriptor *PeakEQDescriptor = NULL;
static LV2_Descriptor *PeakEQCVDescriptor = NULL;

typedef filtType3    PeakEQ;
typedef filtType3_cv PeakEQCV;

static void cleanupPeakEQ(LV2_Handle instance)
{
    free(instance);
}

static void connectPortPeakEQ(
    LV2_Handle instance, uint32_t port, void *data)
{
    PeakEQ *plugin = (PeakEQ *)instance;
    switch(port){
        case 0: plugin->input = data;       break;
        case 1: plugin->output = data;      break;
        case 2: plugin->gain = data;        break;
        case 3: plugin->freq_ofs = data;    break;
        case 4: plugin->freq_pitch = data;  break;
        case 5: plugin->reso_ofs = data;    break;
        case 6: plugin->dBgain_ofs = data;  break;
    }
}

static LV2_Handle instantiatePeakEQ(
    const LV2_Descriptor *descriptor,
    double s_rate,
    const char *path,
    const LV2_Feature * const* features)
{
    PeakEQ* plugin_data = (PeakEQ*)malloc(sizeof(PeakEQ));
    plugin_data->rate = s_rate;
    return (LV2_Handle)plugin_data;
}

static void activatePeakEQ(LV2_Handle instance)
{
    PeakEQ *plugin_data = (PeakEQ *)instance;
    int l1;
    for (l1 = 0; l1 < 4; l1++)
      plugin_data->buf[l1] = 0;
}

static void runPeakEQ(LV2_Handle instance, uint32_t sample_count)
{
    uint32_t l1;
    double f, q, pi2_rate;
    float A;
    double *buf;
    double iv_sin, iv_cos, iv_alpha;
    double inv_a0, a0, a1, a2, b0, b1, b2;
    PeakEQ *pluginData = (PeakEQ *)instance;
    float *input = pluginData->input;
    float *output = pluginData->output;
    float gain = *(pluginData->gain);
    float freq_pitch =
        (*(pluginData->freq_pitch) > 0)
            ? 1.0 + *(pluginData->freq_pitch) / 2.0
            : 1.0 / (1.0 - *(pluginData->freq_pitch) / 2.0);
    float dBgain = *(pluginData->dBgain_ofs);
    pi2_rate = 2.0 * M_PI / pluginData->rate;
    buf = pluginData->buf;
    f =  *(pluginData->freq_ofs) * freq_pitch;
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
    iv_alpha = iv_sin / (Q_SCALE * q);
    A = exp(dBgain / 40.0 * log(10.0));
    b0 = 1.0 + iv_alpha * A;
    b1 = -2.0 * iv_cos;
    b2 = 1.0 - iv_alpha * A;
    a0 = 1.0 + iv_alpha / A;
    a1 = -2.0 * iv_cos;
    a2 = 1.0 - iv_alpha / A;
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

static void initPeakEQ()
{
    PeakEQDescriptor =
        (LV2_Descriptor *) malloc(sizeof(LV2_Descriptor));
    PeakEQDescriptor->URI =              PEAKEQ_URI;
    PeakEQDescriptor->activate =         activatePeakEQ;
    PeakEQDescriptor->cleanup =          cleanupPeakEQ;
    PeakEQDescriptor->connect_port =     connectPortPeakEQ;
    PeakEQDescriptor->deactivate =       NULL;
    PeakEQDescriptor->instantiate =      instantiatePeakEQ;
    PeakEQDescriptor->run =              runPeakEQ;
    PeakEQDescriptor->extension_data =   NULL;
}

static void cleanupPeakEQCV(LV2_Handle instance)
{
    free(instance);
}

static void connectPortPeakEQCV(
    LV2_Handle instance, uint32_t port, void *data)
{
    PeakEQCV *plugin = (PeakEQCV *)instance;
    switch(port){
        case 0: plugin->input = data;       break;
        case 1: plugin->output = data;      break;
        case 2: plugin->gain = data;        break;
        case 3: plugin->freq_ofs = data;    break;
        case 4: plugin->freq_pitch = data;  break;
        case 5: plugin->freq_in = data;     break;
        case 6: plugin->reso_ofs = data;    break;
        case 7: plugin->reso_in = data;     break;
        case 8: plugin->dBgain_ofs = data;  break;
        case 9: plugin->dBgain_in = data;   break;
    }
}

static LV2_Handle instantiatePeakEQCV(
    const LV2_Descriptor *descriptor,
    double s_rate,
    const char *path,
    const LV2_Feature * const* features)
{
    PeakEQCV* plugin_data = (PeakEQCV*)malloc(sizeof(PeakEQCV));
    plugin_data->rate = s_rate;
    return (LV2_Handle)plugin_data;
}

static void activatePeakEQCV(LV2_Handle instance)
{
    PeakEQCV *plugin_data = (PeakEQCV *)instance;
    int l1;
    for (l1 = 0; l1 < 4; l1++)
      plugin_data->buf[l1] = 0;
}

static void runPeakEQCV(LV2_Handle instance, uint32_t sample_count)
{
    uint32_t l1;
    double f0, q0, f, q, pi2_rate;
    float A, dBgain;
    double *buf;
    double iv_sin, iv_cos, iv_alpha;
    double inv_a0, a0, a1, a2, b0, b1, b2;
    PeakEQCV *pluginData = (PeakEQCV *)instance;
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
    float dBgain_ofs = *(pluginData->dBgain_ofs);
    float *dBgain_in = pluginData->dBgain_in;
    pi2_rate = 2.0 * M_PI / pluginData->rate;
    buf = pluginData->buf;
    f0 = freq_ofs;
    q0 = reso_ofs;
    if (!(freq_in || reso_in || dBgain_in)) {
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
        dBgain = dBgain_ofs;
        iv_sin = sin(pi2_rate * f);
        iv_cos = cos(pi2_rate * f);
        iv_alpha = iv_sin/(Q_SCALE * q);
        A = exp(dBgain/40.0 * log(10.0));
        b0 = 1.0 + iv_alpha * A;
        b1 = -2.0 * iv_cos;
        b2 = 1.0 - iv_alpha * A;
        a0 = 1.0 + iv_alpha / A;
        a1 = -2.0 * iv_cos;
        a2 = 1.0 - iv_alpha / A;
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
        if (!reso_in || dBgain_in) {
            q = q0;
            if (q < Q_MIN)
                q = Q_MIN;
            if (q > Q_MAX)
                q = Q_MAX;
            dBgain = dBgain_ofs;
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
                A = exp(dBgain/40.0 * log(10.0));
                b0 = 1.0 + iv_alpha * A;
                b1 = -2.0 * iv_cos;
                b2 = 1.0 - iv_alpha * A;
                a0 = 1.0 + iv_alpha / A;
                a1 = -2.0 * iv_cos;
                a2 = 1.0 - iv_alpha / A;
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
        else {
            for (l1 = 0; l1 < sample_count; l1++) {
                f = (freq_in && (freq_in[l1] > 0))
                    ? (freq_in[l1] * MAX_FREQ + f0 - MIN_FREQ)
                        * freq_pitch
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
                dBgain = (dBgain_in) ? dBgain_ofs + DBGAIN_SCALE * dBgain_in[l1] : dBgain_ofs;
                iv_sin = sin(pi2_rate * f);
                iv_cos = cos(pi2_rate * f);
                iv_alpha = iv_sin/(Q_SCALE * q);
                A = exp(dBgain/40.0 * log(10.0));
                b0 = 1.0 + iv_alpha * A;
                b1 = -2.0 * iv_cos;
                b2 = 1.0 - iv_alpha * A;
                a0 = 1.0 + iv_alpha / A;
                a1 = -2.0 * iv_cos;
                a2 = 1.0 - iv_alpha / A;
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

static void initPeakEQCV()
{
    PeakEQCVDescriptor =
        (LV2_Descriptor *) malloc(sizeof(LV2_Descriptor));
    PeakEQCVDescriptor->URI =              PEAKEQCV_URI;
    PeakEQCVDescriptor->activate =         activatePeakEQCV;
    PeakEQCVDescriptor->cleanup =          cleanupPeakEQCV;
    PeakEQCVDescriptor->connect_port =     connectPortPeakEQCV;
    PeakEQCVDescriptor->deactivate =       NULL;
    PeakEQCVDescriptor->instantiate =      instantiatePeakEQCV;
    PeakEQCVDescriptor->run =              runPeakEQCV;
    PeakEQCVDescriptor->extension_data =   NULL;
}

LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
    switch(index){
        case 0:
            if (!PeakEQDescriptor)
                initPeakEQ();
            return PeakEQDescriptor;
        case 1:
            if (!PeakEQCVDescriptor)
                initPeakEQCV();
            return PeakEQCVDescriptor;
    }
    return NULL;
}
