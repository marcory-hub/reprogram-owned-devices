#include "vespa_detect.hpp"
#include "esp_log.h"
#include "sdkconfig.h"
#include <filesystem>

#if CONFIG_VESPA_DETECT_MODEL_IN_FLASH_RODATA
extern const uint8_t vespa_detect_espdl[] asm("_binary_vespa_detect_espdl_start");
static const char *path = (const char *)vespa_detect_espdl;
#elif CONFIG_VESPA_DETECT_MODEL_IN_FLASH_PARTITION
static const char *path = "vespa_det";
#else
#if !defined(CONFIG_BSP_SD_MOUNT_POINT)
#define CONFIG_BSP_SD_MOUNT_POINT "/sdcard"
#endif
#endif

namespace {

bool model_param_copy_enabled()
{
    return heap_caps_get_total_size(MALLOC_CAP_SPIRAM) >= 1024 * 1024 * 9;
}

dl::Model *create_model(const char *model_name)
{
#if !CONFIG_VESPA_DETECT_MODEL_IN_SDCARD
    const bool param_copy = model_param_copy_enabled();
    return new dl::Model(path,
                         model_name,
                         static_cast<fbs::model_location_type_t>(CONFIG_VESPA_DETECT_MODEL_LOCATION),
                         0,
                         dl::MEMORY_MANAGER_GREEDY,
                         nullptr,
                         param_copy);
#else
    auto sd_path = std::filesystem::path(CONFIG_BSP_SD_MOUNT_POINT) / CONFIG_VESPA_DETECT_MODEL_SDCARD_DIR / model_name;
    return new dl::Model(sd_path.c_str(), fbs::MODEL_LOCATION_IN_SDCARD);
#endif
}

void configure_preprocessor(dl::image::ImagePreprocessor *preprocessor)
{
#if CONFIG_PIPELINE_SKIP_LETTERBOX
    (void)preprocessor;
#else
    preprocessor->enable_letterbox({114, 114, 114});
#endif
}

} // namespace

namespace vespa_detect {
Yolo11n::Yolo11n(const char *model_name, float score_thr, float nms_thr)
{
    m_model = create_model(model_name);
    m_model->minimize();
    m_image_preprocessor = new dl::image::ImagePreprocessor(m_model, {0, 0, 0}, {255, 255, 255});
    configure_preprocessor(m_image_preprocessor);
    m_postprocessor = new dl::detect::yolo11PostProcessor(
        m_model, m_image_preprocessor, score_thr, nms_thr, 10, {{8, 8, 4, 4}, {16, 16, 8, 8}, {32, 32, 16, 16}});
}

ESPDetPico::ESPDetPico(const char *model_name, float score_thr, float nms_thr)
{
    m_model = create_model(model_name);
    m_model->minimize();
#if CONFIG_IDF_TARGET_ESP32P4
    m_image_preprocessor = new dl::image::ImagePreprocessor(m_model, {0, 0, 0}, {255, 255, 255});
#else
    m_image_preprocessor = new dl::image::ImagePreprocessor(m_model, {0, 0, 0}, {255, 255, 255});
#endif
    configure_preprocessor(m_image_preprocessor);
    m_postprocessor = new dl::detect::ESPDetPostProcessor(
        m_model, m_image_preprocessor, score_thr, nms_thr, 10, {{8, 8, 4, 4}, {16, 16, 8, 8}, {32, 32, 16, 16}});
}

} // namespace vespa_detect

VespaDetect::VespaDetect(model_type_t model_type, bool lazy_load) : m_model_type(model_type)
{
    m_score_thr[0] = CONFIG_PIPELINE_SCORE_THR_X100 / 100.0f;
    m_nms_thr[0] = CONFIG_PIPELINE_NMS_THR_X100 / 100.0f;

    if (lazy_load) {
        m_model = nullptr;
    } else {
        load_model();
    }
}

void VespaDetect::load_model()
{
    switch (m_model_type) {
    case model_type_t::YOLO11N_S8_V1:
#if CONFIG_FLASH_VESPA_DETECT_YOLO11N_S8_V1 || CONFIG_VESPA_DETECT_MODEL_IN_SDCARD
        ESP_LOGI("vespa_detect", "loading yolo11n_vespa_2026-05_top.espdl (4 class)");
        m_model = new vespa_detect::Yolo11n("yolo11n_vespa_2026-05_top.espdl", m_score_thr[0], m_nms_thr[0]);
#else
        ESP_LOGE("vespa_detect", "yolo11n_vespa_2026-05_top.espdl is not selected in menuconfig.");
#endif
        break;
    case model_type_t::ESPDET_PICO_VVEL_192x256:
#if CONFIG_FLASH_VESPA_DETECT_ESPDET_PICO_VVEL_192x256 || CONFIG_VESPA_DETECT_MODEL_IN_SDCARD
        ESP_LOGI("vespa_detect", "loading espdet_vespa_top_vvel_vcra_192x256.espdl (2 class)");
        m_model = new vespa_detect::ESPDetPico("espdet_vespa_top_vvel_vcra_192x256.espdl", m_score_thr[0], m_nms_thr[0]);
#else
        ESP_LOGE("vespa_detect", "espdet_vespa_top_vvel_vcra_192x256.espdl is not selected in menuconfig.");
#endif
        break;
    case model_type_t::ESPDET_PICO_VVEL_ONLY_192x256:
#if CONFIG_FLASH_VESPA_DETECT_ESPDET_PICO_VVEL_ONLY_192x256 || CONFIG_VESPA_DETECT_MODEL_IN_SDCARD
        ESP_LOGI("vespa_detect", "loading espdet_vespa_top_vvel_192x256.espdl (1 class)");
        m_model = new vespa_detect::ESPDetPico("espdet_vespa_top_vvel_192x256.espdl", m_score_thr[0], m_nms_thr[0]);
#else
        ESP_LOGE("vespa_detect", "espdet_vespa_top_vvel_192x256.espdl is not selected in menuconfig.");
#endif
        break;
    case model_type_t::ESPDET_PICO_S8_V1:
        ESP_LOGE("vespa_detect", "espdet_pico_s8_v1 removed from flash bundle; pick 4/2/1 class on boot.");
        break;
    }
}
