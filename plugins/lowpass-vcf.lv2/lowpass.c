/******************* Lowpass, Formula by Robert Bristow-Johnson ********************/
/*  LADSPA version by Matthias Nagorni
    LV2 port by James W. Morris <james@jwm-art.net>
*/

#include <stdlib.h>
#include <math.h>
#include <lv2.h>

#include "vcf.h"
#include "filter_type2.h"

#define LOWPASS_URI   "http://jwm-art.net/lv2/vcf/lowpass";
#define LOWPASSCV_URI "http://jwm-art.net/lv2/vcf/lowpass_cv";

static LV2_Descriptor *LowpassDescriptor = NULL;
static LV2_Descriptor *LowpassCVDescriptor = NULL;

typedef filtType2    Lowpass;
typedef filtType2_cv LowpassCV;

static void cleanupLowpass(LV2_Handle instance)
{
    free(instance);
}

static void connectPortLowpass(
    LV2_Handle instance, uint32_t port, void *data)
{
    Lowpass *plugin = (Lowpass *)instance;
    switch(port){
        case 0: plugin->input = data;       break;
        case 1: plugin->output = data;      break;
        case 2: plugin->gain = data;        break;
        case 3: plugin->freq_ofs = data;    break;
        case 4: plugin->freq_pitch = data;  break;
        case 5: plugin->reso_ofs = data;    break;
    }
}

static LV2_Handle instantiateLowpass(
    const LV2_Descriptor *descriptor,
    double s_rate,
    const char *path,
    const LV2_Feature * const* features)
{
    Lowpass* plugin_data = (Lowpass*)malloc(sizeof(Lowpass));
    plugin_data->rate = s_rate;
    return (LV2_Handle)plugin_data;
}

static void activateLowpass(LV2_Handle instance)
{
    Lowpass *plugin_data = (Lowpass *)instance;
    int l1;
    for (l1 = 0; l1 < 4; l1++)
      plugin_data->buf[l1] = 0;
}

static void runLowpass(LV2_Handle instance, uint32_t sample_count)
{
    uint32_t l1;
    double f, q, pi2_rate;
    double *buf;
    double iv_sin, iv_cos, iv_alpha;
    double inv_a0, a0, a1, a2, b0, b1, b2;
    Lowpass *pluginData = (Lowpass *)instance;
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
    b0 = (1.0 - iv_cos) / 2.0;
    b1 = 1.0 - iv_cos;
    b2 = b0;
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

static void initLowpass()
{
    LowpassDescriptor =
        (LV2_Descriptor *) malloc(sizeof(LV2_Descriptor));
    LowpassDescriptor->URI =              LOWPASS_URI;
    LowpassDescriptor->activate =         activateLowpass;
    LowpassDescriptor->cleanup =          cleanupLowpass;
    LowpassDescriptor->connect_port =     connectPortLowpass;
    LowpassDescriptor->deactivate =       NULL;
    LowpassDescriptor->instantiate =      instantiateLowpass;
    LowpassDescriptor->run =              runLowpass;
    LowpassDescriptor->extension_data =   NULL;
}

static void cleanupLowpassCV(LV2_Handle instance)
{
    free(instance);
}

static void connectPortLowpassCV(
    LV2_Handle instance, uint32_t port, void *data)
{
    LowpassCV *plugin = (LowpassCV *)instance;
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

static LV2_Handle instantiateLowpassCV(
    const LV2_Descriptor *descriptor,
    double s_rate,
    const char *path,
    const LV2_Feature * const* features)
{
    LowpassCV* plugin_data = (LowpassCV*)malloc(sizeof(LowpassCV));
    plugin_data->rate = s_rate;
    return (LV2_Handle)plugin_data;
}

static void activateLowpassCV(LV2_Handle instance)
{
    LowpassCV *plugin_data = (LowpassCV *)instance;
    int l1;
    for (l1 = 0; l1 < 4; l1++)
      plugin_data->buf[l1] = 0;
}

static void runLowpassCV(LV2_Handle instance, uint32_t sample_count)
{
    uint32_t l1;
    double f0, q0, f, q, pi2_rate;
    double *buf;
    double iv_sin, iv_cos, iv_alpha;
    double inv_a0, a0, a1, a2, b0, b1, b2;
    LowpassCV *pluginData = (LowpassCV *)instance;
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
        b0 = (1.0 - iv_cos) / 2.0;
        b1 = 1.0 - iv_cos;
        b2 = b0;
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
                b0 = (1.0 - iv_cos) / 2.0;
                b1 = 1.0 - iv_cos;
                b2 = b0;
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
                iv_sin = sin(pi2_rate * f);
                iv_cos = cos(pi2_rate * f);
                iv_alpha = iv_sin/(Q_SCALE * q);
                b0 = (1.0 - iv_cos) / 2.0;
                b1 = 1.0 - iv_cos;
                b2 = b0;
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

static void initLowpassCV()
{
    LowpassCVDescriptor =
        (LV2_Descriptor *) malloc(sizeof(LV2_Descriptor));
    LowpassCVDescriptor->URI =              LOWPASSCV_URI;
    LowpassCVDescriptor->activate =         activateLowpassCV;
    LowpassCVDescriptor->cleanup =          cleanupLowpassCV;
    LowpassCVDescriptor->connect_port =     connectPortLowpassCV;
    LowpassCVDescriptor->deactivate =       NULL;
    LowpassCVDescriptor->instantiate =      instantiateLowpassCV;
    LowpassCVDescriptor->run =              runLowpassCV;
    LowpassCVDescriptor->extension_data =   NULL;
}

LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
    switch(index){
        case 0:
            if (!LowpassDescriptor)
                initLowpass();
            return LowpassDescriptor;
        case 1:
            if (!LowpassCVDescriptor)
                initLowpassCV();
            return LowpassCVDescriptor;
    }
    return NULL;
}
