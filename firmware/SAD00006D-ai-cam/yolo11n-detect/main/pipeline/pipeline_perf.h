#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PIPE_STAGE_CAPTURE = 0,
    PIPE_STAGE_CONVERT,
    PIPE_STAGE_RESIZE,
    PIPE_STAGE_INFER,
    PIPE_STAGE_LCD,
    PIPE_STAGE_JPEG,
    PIPE_STAGE_SD_QUEUE,
    PIPE_STAGE_OUTPUT,
    PIPE_STAGE_E2E,
    PIPE_STAGE_COUNT
} pipe_stage_t;

typedef struct {
    int64_t us;
    uint32_t seq;
} pipe_stage_sample_t;

/** Monotonic timestamp in microseconds. */
int64_t pipe_perf_now_us(void);

/** Reset rolling stats and sequence counter. */
void pipe_perf_reset(void);

/** Record one stage duration for the current frame. */
void pipe_perf_record(pipe_stage_t stage, int64_t duration_us);

/** Attach the current frame sequence id to the next total sample. */
void pipe_perf_set_seq(uint32_t seq);

/** Log rolling average/min/max for each stage (every log_every frames). */
void pipe_perf_log_if_due(uint32_t log_every);

/** Human-readable stage label. */
const char *pipe_perf_stage_name(pipe_stage_t stage);

#ifdef __cplusplus
}
#endif
