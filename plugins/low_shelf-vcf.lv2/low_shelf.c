/******************* LowShelf, Formula by Robert Bristow-Johnson ********************/
/*  LADSPA version by Matthias Nagorni
    LV2 port by James W. Morris <james@jwm-art.net>
*/

#include <stdlib.h>
#include <math.h>
#include <lv2.h>

#include "vcf.h"
#include "filter_type3.h"

#define LOWSHELF_URI   "http://jwm-art.net/lv2/vcf/low_shelf";
#define LOWSHELFCV_URI "http://jwm-art.net/lv2/vcf/low_shelf_cv";

static LV2_Descriptor *LowShelfDescriptor = NULL;
static LV2_Descriptor *LowShelfCVDescriptor = NULL;

typedef filtType3    LowShelf;
typedef filtType3_cv LowShelfCV;

static void cleanupLowShelf(LV2_Handle instance)
{
    free(instance);
}

static void connectPortLowShelf(
    LV2_Handle instance, uint32_t port, void *data)
{
    LowShelf *plugin = (LowShelf *)instance;
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

static LV2_Handle instantiateLowShelf(
    const LV2_Descriptor *descriptor,
    double s_rate,
    const char *path,
    const LV2_Feature * const* features)
{
    LowShelf* plugin_data = (LowShelf*)malloc(sizeof(LowShelf));
    plugin_data->rate = s_rate;
    return (LV2_Handle)plugin_data;
}

static void activateLowShelf(LV2_Handle instance)
{
    LowShelf *plugin_data = (LowShelf *)instance;
    int l1;
    for (l1 = 0; l1 < 4; l1++)
      plugin_data->buf[l1] = 0;
}

static void runLowShelf(LV2_Handle instance, uint32_t sample_count)
{
    uint32_t l1;
    float out;
    double f, q, pi2_rate;
    float A, dBgain, iv_beta;
    double *buf;
    double iv_sin, iv_cos, iv_alpha;
    double inv_a0, a0, a1, a2, b0, b1, b2;
    LowShelf *pluginData = (LowShelf *)instance;
    float *input = pluginData->input;
    float *output = pluginData->output;
    float gain = *(pluginData->gain);
    float freq_pitch =
        (*(pluginData->freq_pitch) > 0)
            ? 1.0 + *(pluginData->freq_pitch) / 2.0
            : 1.0 / (1.0 - *(pluginData->freq_pitch) / 2.0);
    float dBgain_ofs = *(pluginData->dBgain_ofs);
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
    dBgain = dBgain_ofs;
    iv_sin = sin(pi2_rate * f);
    iv_cos = cos(pi2_rate * f);
    iv_alpha = iv_sin / (Q_SCALE * q);
    A = exp(dBgain / 40.0 * log(10.0));
    iv_beta = sqrt(A) / q;
    b0 = A * (A + 1.0 - (A - 1.0) * iv_cos + iv_beta * iv_sin);
    b1 = 2.0 * A * (A - 1.0 - (A + 1.0) * iv_cos);
    b2 = A * (A + 1.0 - (A - 1.0) * iv_cos - iv_beta * iv_sin);
    a0 = A + 1.0 + (A - 1.0) * iv_cos + iv_beta * iv_sin;
    a1 = -2.0 * (A - 1.0 + (A + 1.0) * iv_cos);
    a2 = A + 1.0 + (A - 1.0) * iv_cos - iv_beta * iv_sin;
    inv_a0 = 1.0 / a0;
    for (l1 = 0; l1 < sample_count; l1++) {
        out = inv_a0 * (gain
            * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                - a1 * buf[2] - a2 * buf[3]);
        buf[1] = buf[0];
        buf[0] = input[l1];
        buf[3] = buf[2];
        buf[2] = output[l1] = out;
    }
}

static void initLowShelf()
{
    LowShelfDescriptor =
        (LV2_Descriptor *) malloc(sizeof(LV2_Descriptor));
    LowShelfDescriptor->URI =              LOWSHELF_URI;
    LowShelfDescriptor->activate =         activateLowShelf;
    LowShelfDescriptor->cleanup =          cleanupLowShelf;
    LowShelfDescriptor->connect_port =     connectPortLowShelf;
    LowShelfDescriptor->deactivate =       NULL;
    LowShelfDescriptor->instantiate =      instantiateLowShelf;
    LowShelfDescriptor->run =              runLowShelf;
    LowShelfDescriptor->extension_data =   NULL;
}

static void cleanupLowShelfCV(LV2_Handle instance)
{
    free(instance);
}

static void connectPortLowShelfCV(
    LV2_Handle instance, uint32_t port, void *data)
{
    LowShelfCV *plugin = (LowShelfCV *)instance;
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

static LV2_Handle instantiateLowShelfCV(
    const LV2_Descriptor *descriptor,
    double s_rate,
    const char *path,
    const LV2_Feature * const* features)
{
    LowShelfCV* plugin_data = (LowShelfCV*)malloc(sizeof(LowShelfCV));
    plugin_data->rate = s_rate;
    return (LV2_Handle)plugin_data;
}

static void activateLowShelfCV(LV2_Handle instance)
{
    LowShelfCV *plugin_data = (LowShelfCV *)instance;
    int l1;
    for (l1 = 0; l1 < 4; l1++)
      plugin_data->buf[l1] = 0;
}

static void runLowShelfCV(LV2_Handle instance, uint32_t sample_count)
{
    uint32_t l1;
    float out;
    double f0, q0, f, q, pi2_rate;
    float A, dBgain, iv_beta;
    double *buf;
    double iv_sin, iv_cos, iv_alpha;
    double inv_a0, a0, a1, a2, b0, b1, b2;
    LowShelfCV *pluginData = (LowShelfCV *)instance;
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
        iv_beta = sqrt(A) / q;
        b0 = A * (A + 1.0 - (A - 1.0) * iv_cos + iv_beta * iv_sin);
        b1 = 2.0 * A * (A - 1.0 - (A + 1.0) * iv_cos);
        b2 = A * (A + 1.0 - (A - 1.0) * iv_cos - iv_beta * iv_sin);
        a0 = A + 1.0 + (A - 1.0) * iv_cos + iv_beta * iv_sin;
        a1 = -2.0 * (A - 1.0 + (A + 1.0) * iv_cos);
        a2 = A + 1.0 + (A - 1.0) * iv_cos - iv_beta * iv_sin;
        inv_a0 = 1.0 / a0;
        for (l1 = 0; l1 < sample_count; l1++) {
            out = inv_a0 * (gain
                * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                    - a1 * buf[2] - a2 * buf[3]);
            buf[1] = buf[0];
            buf[0] = input[l1];
            buf[3] = buf[2];
            buf[2] = output[l1] = out;
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
                iv_beta = sqrt(A) / q;
                b0 = A * (A + 1.0 - (A - 1.0) * iv_cos + iv_beta * iv_sin);
                b1 = 2.0 * A * (A - 1.0 - (A + 1.0) * iv_cos);
                b2 = A * (A + 1.0 - (A - 1.0) * iv_cos - iv_beta * iv_sin);
                a0 = A + 1.0 + (A - 1.0) * iv_cos + iv_beta * iv_sin;
                a1 = -2.0 * (A - 1.0 + (A + 1.0) * iv_cos);
                a2 = A + 1.0 + (A - 1.0) * iv_cos - iv_beta * iv_sin;
                out = 1.0 / a0 * (gain
                    * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                        - a1 * buf[2] - a2 * buf[3]);
                buf[1] = buf[0];
                buf[0] = input[l1];
                buf[3] = buf[2];
                buf[2] = output[l1] = out;
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
                iv_beta = sqrt(A) / q;
                b0 = A * (A + 1.0 - (A - 1.0) * iv_cos + iv_beta * iv_sin);
                b1 = 2.0 * A * (A - 1.0 - (A + 1.0) * iv_cos);
                b2 = A * (A + 1.0 - (A - 1.0) * iv_cos - iv_beta * iv_sin);
                a0 = A + 1.0 + (A - 1.0) * iv_cos + iv_beta * iv_sin;
                a1 = -2.0 * (A - 1.0 + (A + 1.0) * iv_cos);
                a2 = A + 1.0 + (A - 1.0) * iv_cos - iv_beta * iv_sin;
                inv_a0 = 1.0 / a0;
                out = inv_a0 * (gain
                    * (b0 * input[l1] + b1 * buf[0] + b2 * buf[1])
                        - a1 * buf[2] - a2 * buf[3]);
                buf[1] = buf[0];
                buf[0] = input[l1];
                buf[3] = buf[2];
                buf[2] = output[l1] = out;
            }
        }
    }
}

static void initLowShelfCV()
{
    LowShelfCVDescriptor =
        (LV2_Descriptor *) malloc(sizeof(LV2_Descriptor));
    LowShelfCVDescriptor->URI =              LOWSHELFCV_URI;
    LowShelfCVDescriptor->activate =         activateLowShelfCV;
    LowShelfCVDescriptor->cleanup =          cleanupLowShelfCV;
    LowShelfCVDescriptor->connect_port =     connectPortLowShelfCV;
    LowShelfCVDescriptor->deactivate =       NULL;
    LowShelfCVDescriptor->instantiate =      instantiateLowShelfCV;
    LowShelfCVDescriptor->run =              runLowShelfCV;
    LowShelfCVDescriptor->extension_data =   NULL;
}

LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
    switch(index){
        case 0:
            if (!LowShelfDescriptor)
                initLowShelf();
            return LowShelfDescriptor;
        case 1:
            if (!LowShelfCVDescriptor)
                initLowShelfCV();
            return LowShelfCVDescriptor;
    }
    return NULL;
}
