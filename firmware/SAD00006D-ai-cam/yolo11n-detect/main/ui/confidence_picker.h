#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Second boot screen after model pick. Blocks until touch or 60 s timeout.
 * Options: >0.60, >0.70, >0.80. Default on timeout: >0.70.
 * @return score threshold as float (0.60, 0.70, or 0.80)
 */
float confidence_picker_run(void);

#ifdef __cplusplus
}
#endif
