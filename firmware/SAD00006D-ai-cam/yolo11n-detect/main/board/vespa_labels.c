#include "vespa_labels.h"

#include <stddef.h>

/* Matches VespaDetect::model_type_t values. */
enum {
    VESPA_MODEL_YOLO11N = 0,
    VESPA_MODEL_ESPDET_224 = 1,
    VESPA_MODEL_ESPDET_VVEL_VCRA = 2,
    VESPA_MODEL_ESPDET_VVEL_ONLY = 3,
};

static int s_model_type = VESPA_MODEL_ESPDET_VVEL_VCRA;

void vespa_labels_set_model(int model_type)
{
    s_model_type = model_type;
}

const char *vespa_labels_class_name(int category)
{
    switch (s_model_type) {
    case VESPA_MODEL_ESPDET_VVEL_ONLY:
        return (category == 0) ? "vvel" : "?";
    case VESPA_MODEL_ESPDET_VVEL_VCRA:
        switch (category) {
        case 0:
            return "vvel";
        case 1:
            return "vcra";
        default:
            return "?";
        }
    case VESPA_MODEL_YOLO11N:
    case VESPA_MODEL_ESPDET_224:
    default:
        switch (category) {
        case 0:
            return "amel";
        case 1:
            return "vcra";
        case 2:
            return "vespsp";
        case 3:
            return "vvel";
        default:
            return "?";
        }
    }
}

void vespa_labels_infer_size(int model_type, int *out_w, int *out_h)
{
    if (out_w == NULL || out_h == NULL) {
        return;
    }
    switch (model_type) {
    case VESPA_MODEL_ESPDET_VVEL_VCRA:
    case VESPA_MODEL_ESPDET_VVEL_ONLY:
        *out_w = 256;
        *out_h = 192;
        break;
    case VESPA_MODEL_YOLO11N:
    case VESPA_MODEL_ESPDET_224:
    default:
        *out_w = 224;
        *out_h = 224;
        break;
    }
}
