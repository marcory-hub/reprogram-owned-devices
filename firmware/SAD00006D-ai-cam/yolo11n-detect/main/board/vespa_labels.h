#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Model type integers match VespaDetect::model_type_t:
 * 0 YOLO11n 4-class, 1 ESPDet-224, 2 ESPDet 2-class 192x256, 3 ESPDet 1-class 192x256.
 */
void vespa_labels_set_model(int model_type);

/** Class short name for LCD / SD filenames. */
const char *vespa_labels_class_name(int category);

/** Infer size for model_type (W x H). */
void vespa_labels_infer_size(int model_type, int *out_w, int *out_h);

#ifdef __cplusplus
}
#endif
