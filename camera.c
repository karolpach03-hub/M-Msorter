#include "header.h"

esp_err_t init_cam()
{
    esp_err_t err=esp_camera_init(&camera_config);
    if(err!=ESP_OK) 
    {
        ESP_LOGE("CAM", "Camera init failed with error 0x%x", err);
        return err;
    }
    ESP_LOGI("CAM", "Camera initialized successfully");
    return ESP_OK;
}