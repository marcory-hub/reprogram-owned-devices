#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate buffers, detector, and FreeRTOS queues.
 * @param model_type VespaDetect::model_type_t value (0..3).
 * @param conf_thr LCD status / box confidence gate (e.g. 0.60, 0.70, 0.80).
 */
esp_err_t detect_pipeline_init(int model_type, float conf_thr);

/** Run boot smoke tests (jpeg + one camera frame). */
esp_err_t detect_pipeline_smoke(void);

/** Start dual-core capture / infer / output tasks. Does not return. */
void detect_pipeline_start(void);

#ifdef __cplusplus
}
#endif
