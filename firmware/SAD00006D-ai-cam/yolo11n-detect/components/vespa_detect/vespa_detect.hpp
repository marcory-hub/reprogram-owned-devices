#pragma once
#include "dl_detect_base.hpp"
#include "dl_detect_espdet_postprocessor.hpp"
#include "dl_detect_yolo11_postprocessor.hpp"

namespace vespa_detect {
class Yolo11n : public dl::detect::DetectImpl {
public:
    static constexpr float default_score_thr = 0.25;
    static constexpr float default_nms_thr = 0.7;
    Yolo11n(const char *model_name, float score_thr, float nms_thr);
};

class ESPDetPico : public dl::detect::DetectImpl {
public:
    static constexpr float default_score_thr = 0.25;
    static constexpr float default_nms_thr = 0.7;
    ESPDetPico(const char *model_name, float score_thr, float nms_thr);
};
} // namespace vespa_detect

class VespaDetect : public dl::detect::DetectWrapper {
public:
    typedef enum {
        YOLO11N_S8_V1 = 0,
        ESPDET_PICO_S8_V1 = 1,
        /** 2-class: 0=vvel, 1=vcra @ 192x256. */
        ESPDET_PICO_VVEL_192x256 = 2,
        /** 1-class: 0=vvel @ 192x256. */
        ESPDET_PICO_VVEL_ONLY_192x256 = 3,
    } model_type_t;
    VespaDetect(model_type_t model_type = static_cast<model_type_t>(CONFIG_DEFAULT_VESPA_DETECT_MODEL),
                bool lazy_load = true);

    model_type_t model_type() const { return m_model_type; }

private:
    void load_model() override;

    model_type_t m_model_type;
};
