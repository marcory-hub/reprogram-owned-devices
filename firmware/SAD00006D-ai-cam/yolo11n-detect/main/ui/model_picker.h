#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Full-screen boot picker. Blocks until a touch selects a model or 60 s timeout.
 * Default on timeout: 2-class ESPDet (VespaDetect::ESPDET_PICO_VVEL_192x256 = 2).
 * @return VespaDetect::model_type_t as int
 */
int model_picker_run(void);

#ifdef __cplusplus
}
#endif
