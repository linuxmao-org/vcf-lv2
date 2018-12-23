/****************** Resonant ResLowpass, Formula by Paul Kellett **********************/
/*  LADSPA version by Matthias Nagorni
    LV2 port by James W. Morris <james@jwm-art.net>
*/

#include <stdlib.h>
#include <math.h>
#include <lv2.h>

#include "vcf.h"
#include "filter_type1.h"

#define RESLOWPASS_URI   "http://jwm-art.net/lv2/vcf/resonant_lowpass";
#define RESLOWPASSCV_URI "http://jwm-art.net/lv2/vcf/resonant_lowpass_cv";

static LV2_Descriptor *ResLowpassDescriptor = NULL;
static LV2_Descriptor *ResLowpassCVDescriptor = NULL;

typedef filtType1    ResLowpass;
typedef filtType1_cv ResLowpassCV;

static void cleanupResLowpass(LV2_Handle instance)
{
    free(instance);
}

static void connectPortResLowpass(
    LV2_Handle instance, uint32_t port, void *data)
{
    ResLowpass *plugin = (ResLowpass *)instance;
    switch(port){
        case 0: plugin->input = data;       break;
        case 1: plugin->output = data;      break;
        case 2: plugin->gain = data;        break;
        case 3: plugin->freq_ofs = data;    break;
        case 4: plugin->freq_pitch = data;  break;
        case 5: plugin->reso_ofs = data;    break;
    }
}

static LV2_Handle instantiateResLowpass(
    const LV2_Descriptor *descriptor,
    double s_rate,
    const char *path,
    const LV2_Feature * const* features)
{
    ResLowpass* plugin_data = (ResLowpass*)malloc(sizeof(ResLowpass));
    plugin_data->rate = s_rate;
    return (LV2_Handle)plugin_data;
}

static void activateResLowpass(LV2_Handle instance)
{
    ResLowpass *plugin_data = (ResLowpass *)instance;
    int l1;
    for (l1 = 0; l1 < 2; l1++)
      plugin_data->buf[l1] = 0;
}

static void runResLowpass(LV2_Handle instance, uint32_t sample_count)
{
    uint32_t l1;
    double f0, f, q, fa, fb, rate_f;
    double *buf;
    ResLowpass *pluginData = (ResLowpass *)instance;
    float *input = pluginData->input;
    float *output = pluginData->output;
    float gain = *(pluginData->gain);
    float freq_pitch =
        (*(pluginData->freq_pitch) > 0)
            ? 1.0 + *(pluginData->freq_pitch) / 2.0
            : 1.0 / (1.0 - *(pluginData->freq_pitch) / 2.0);
    rate_f = 44100.0 / pluginData->rate;
    buf = pluginData->buf;
    f0 = *(pluginData->freq_ofs) / (double)MAX_FREQ * rate_f * 2.85;;
    f = f0 * freq_pitch;
    if (f < 0)
        f = 0;
    if (f > 0.9999)
        f = 0.9999;
    q = *(pluginData->reso_ofs);
    if (q < Q_MIN)
        q = Q_MIN;
    if (q > Q_MAX)
        q = Q_MAX;
    fa = 1.0 - f;
    fb = q * (1.0 + (1.0 / fa));
    for (l1 = 0; l1 < sample_count; l1++) {
        buf[0] = fa * buf[0] + f * (input[l1] + fb * (buf[0] - buf[1]));
        buf[1] = fa * buf[1] + f * buf[0];
        output[l1] = gain * buf[1];
    }
}

static void initResLowpass()
{
    ResLowpassDescriptor =
        (LV2_Descriptor *) malloc(sizeof(LV2_Descriptor));
    ResLowpassDescriptor->URI =              RESLOWPASS_URI;
    ResLowpassDescriptor->activate =         activateResLowpass;
    ResLowpassDescriptor->cleanup =          cleanupResLowpass;
    ResLowpassDescriptor->connect_port =     connectPortResLowpass;
    ResLowpassDescriptor->deactivate =       NULL;
    ResLowpassDescriptor->instantiate =      instantiateResLowpass;
    ResLowpassDescriptor->run =              runResLowpass;
    ResLowpassDescriptor->extension_data =   NULL;
}

static void cleanupResLowpassCV(LV2_Handle instance)
{
    free(instance);
}

static void connectPortResLowpassCV(
    LV2_Handle instance, uint32_t port, void *data)
{
    ResLowpassCV *plugin = (ResLowpassCV *)instance;
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

static LV2_Handle instantiateResLowpassCV(
    const LV2_Descriptor *descriptor,
    double s_rate,
    const char *path,
    const LV2_Feature * const* features)
{
    ResLowpassCV* plugin_data = (ResLowpassCV*)malloc(sizeof(ResLowpassCV));
    plugin_data->rate = s_rate;
    return (LV2_Handle)plugin_data;
}

static void activateResLowpassCV(LV2_Handle instance)
{
    ResLowpassCV *plugin_data = (ResLowpassCV *)instance;
    int l1;
    for (l1 = 0; l1 < 2; l1++)
      plugin_data->buf[l1] = 0;
}

static void runResLowpassCV(LV2_Handle instance, uint32_t sample_count)
{
    uint32_t l1;
    double f0, q0, f, q, fa, fb, rate, rate_f, k;
    double *buf;
    ResLowpassCV *pluginData = (ResLowpassCV *)instance;
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
    rate = pluginData->rate;
    rate_f = 44100.0 / rate;
    buf = pluginData->buf;
    f0 = freq_ofs / (double)MAX_FREQ * rate_f * 2.85;;
    q0 = reso_ofs;
    if (!(freq_in || reso_in)) {
        f = f0 * freq_pitch;
        if (f < 0)
            f = 0;
        if (f > 0.9999)
            f = 0.9999;
        q = q0;
        if (q < Q_MIN)
            q = Q_MIN;
        if (q > Q_MAX)
            q = Q_MAX;
        fa = 1.0 - f;
        fb = q * (1.0 + (1.0 / fa));
        for (l1 = 0; l1 < sample_count; l1++) {
            buf[0] = fa * buf[0] + f * (input[l1] + fb * (buf[0] - buf[1]));
            buf[1] = fa * buf[1] + f * buf[0];
            output[l1] = gain * buf[1];
        }
    }
    else {
        if (!reso_in) {
            q = q0;
            if (q < Q_MIN)
                q = Q_MIN;
            if (q > Q_MAX)
                q = Q_MAX;
            k =  MAX_FREQ * 2.85;
            for (l1 = 0; l1 < sample_count; l1++) {
                f = (freq_in[l1] > 0)
                    ? (freq_in[l1] * k + (freq_ofs - MIN_FREQ))
                        / (double)MAX_FREQ * freq_pitch * rate_f
                    : f0 * freq_pitch;
                if (f < 0)
                    f = 0;
                if (f > 0.99)
                    f = 0.99;
                fa = 1.0 - f;
                fb = q * (1.0 + (1.0 / fa));
                buf[0] = fa * buf[0] + f * (input[l1] + fb * (buf[0] - buf[1])); 
                buf[1] = fa * buf[1] + f * buf[0];
                output[l1] = gain * buf[1];
            }
        }
        else {
            k = MAX_FREQ * 2.85;
            for (l1 = 0; l1 < sample_count; l1++) {
                f = (freq_in && (freq_in[l1] > 0))
                    ? (freq_in[l1] * k + (freq_ofs - MIN_FREQ))
                        / (double)MAX_FREQ * freq_pitch * rate_f
                    : f0 * freq_pitch;
                if (f < 0)
                    f = 0;
                if (f > 0.99)
                    f = 0.99;
                q = q0 + reso_in[l1];
                if (q < 0)
                    q = 0;
                if (q > 1)
                    q = 1;
                fa = 1.0 - f;
                fb = q * (1.0 + (1.0 / fa));
                buf[0] = fa * buf[0] + f * (input[l1] + fb * (buf[0] - buf[1]));
                buf[1] = fa * buf[1] + f * buf[0];
                output[l1] = gain * buf[1];
            }
        }
    }
}

static void initResLowpassCV()
{
    ResLowpassCVDescriptor =
        (LV2_Descriptor *) malloc(sizeof(LV2_Descriptor));
    ResLowpassCVDescriptor->URI =              RESLOWPASSCV_URI;
    ResLowpassCVDescriptor->activate =         activateResLowpassCV;
    ResLowpassCVDescriptor->cleanup =          cleanupResLowpassCV;
    ResLowpassCVDescriptor->connect_port =     connectPortResLowpassCV;
    ResLowpassCVDescriptor->deactivate =       NULL;
    ResLowpassCVDescriptor->instantiate =      instantiateResLowpassCV;
    ResLowpassCVDescriptor->run =              runResLowpassCV;
    ResLowpassCVDescriptor->extension_data =   NULL;
}

LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
    switch(index){
        case 0:
            if (!ResLowpassDescriptor)
                initResLowpass();
            return ResLowpassDescriptor;
        case 1:
            if (!ResLowpassCVDescriptor)
                initResLowpassCV();
            return ResLowpassCVDescriptor;
    }
    return NULL;
}
