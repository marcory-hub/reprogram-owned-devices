#include "pipeline_perf.h"

#include "esp_log.h"
#include "esp_timer.h"
#include <limits.h>

static const char *TAG = "pipe_perf";

typedef struct {
    int64_t sum_us;
    int64_t min_us;
    int64_t max_us;
    uint32_t count;
} pipe_stage_stats_t;

static pipe_stage_stats_t s_stats[PIPE_STAGE_COUNT];
static uint32_t s_frame_count;
static uint32_t s_current_seq;

int64_t pipe_perf_now_us(void)
{
    return esp_timer_get_time();
}

void pipe_perf_reset(void)
{
    for (int i = 0; i < PIPE_STAGE_COUNT; i++) {
        s_stats[i].sum_us = 0;
        s_stats[i].min_us = INT64_MAX;
        s_stats[i].max_us = 0;
        s_stats[i].count = 0;
    }
    s_frame_count = 0;
    s_current_seq = 0;
}

void pipe_perf_set_seq(uint32_t seq)
{
    s_current_seq = seq;
}

void pipe_perf_record(pipe_stage_t stage, int64_t duration_us)
{
    if (stage >= PIPE_STAGE_COUNT || duration_us < 0) {
        return;
    }

    pipe_stage_stats_t *st = &s_stats[stage];
    st->sum_us += duration_us;
    if (duration_us < st->min_us) {
        st->min_us = duration_us;
    }
    if (duration_us > st->max_us) {
        st->max_us = duration_us;
    }
    st->count++;

    if (stage == PIPE_STAGE_E2E) {
        s_frame_count++;
    }
}

void pipe_perf_log_if_due(uint32_t log_every)
{
    if (log_every == 0 || s_frame_count == 0 || (s_frame_count % log_every) != 0) {
        return;
    }

    const int64_t e2e_avg = s_stats[PIPE_STAGE_E2E].count > 0
                                ? s_stats[PIPE_STAGE_E2E].sum_us / (int64_t)s_stats[PIPE_STAGE_E2E].count
                                : 0;
    const float fps = e2e_avg > 0 ? 1000000.0f / (float)e2e_avg : 0.0f;

    ESP_LOGI(TAG,
             "frame seq=%u e2e_avg=%lld us (~%.2f FPS) over %u frames",
             (unsigned)s_current_seq,
             (long long)e2e_avg,
             fps,
             (unsigned)s_frame_count);

    for (int i = 0; i < PIPE_STAGE_COUNT; i++) {
        const pipe_stage_stats_t *st = &s_stats[i];
        if (st->count == 0) {
            continue;
        }
        ESP_LOGI(TAG,
                 "  %s avg=%lld min=%lld max=%lld us (n=%u)",
                 pipe_perf_stage_name((pipe_stage_t)i),
                 (long long)(st->sum_us / (int64_t)st->count),
                 (long long)st->min_us,
                 (long long)st->max_us,
                 (unsigned)st->count);
    }
}

const char *pipe_perf_stage_name(pipe_stage_t stage)
{
    switch (stage) {
    case PIPE_STAGE_CAPTURE:
        return "capture";
    case PIPE_STAGE_CONVERT:
        return "convert";
    case PIPE_STAGE_RESIZE:
        return "resize";
    case PIPE_STAGE_INFER:
        return "infer";
    case PIPE_STAGE_LCD:
        return "lcd";
    case PIPE_STAGE_JPEG:
        return "jpeg";
    case PIPE_STAGE_SD_QUEUE:
        return "sd_queue";
    case PIPE_STAGE_OUTPUT:
        return "output";
    case PIPE_STAGE_E2E:
        return "e2e";
    default:
        return "?";
    }
}
