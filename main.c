#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_camera.h"
#include "driver/uart.h"

#define CAM_PIN_PWDN    -1
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK    21
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27

#define CAM_PIN_D7      35
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      19
#define CAM_PIN_D2      18
#define CAM_PIN_D1       5
#define CAM_PIN_D0       4
#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22

static camera_config_t camera_config={
    .pin_pwdn=CAM_PIN_PWDN,
    .pin_reset=CAM_PIN_RESET,
    .pin_xclk=CAM_PIN_XCLK,
    .pin_sscb_sda=CAM_PIN_SIOD,
    .pin_sscb_scl=CAM_PIN_SIOC,
    .pin_d7=CAM_PIN_D7,
    .pin_d6=CAM_PIN_D6,
    .pin_d5=CAM_PIN_D5,
    .pin_d4=CAM_PIN_D4,
    .pin_d3=CAM_PIN_D3,
    .pin_d2=CAM_PIN_D2,
    .pin_d1=CAM_PIN_D1,
    .pin_d0=CAM_PIN_D0,
    .pin_vsync=CAM_PIN_VSYNC,
    .pin_href=CAM_PIN_HREF,
    .pin_pclk=CAM_PIN_PCLK,
    .xclk_freq_hz=20000000,
    .ledc_timer=LEDC_TIMER_0,
    .ledc_channel=LEDC_CHANNEL_0,
    .pixel_format=PIXFORMAT_RGB565,
    .frame_size=FRAMESIZE_128X128,
    .fb_count=1,
    .fb_location=CAMERA_FB_IN_DRAM,
    .grab_mode=CAMERA_GRAB_WHEN_EMPTY
};

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

uart_config_t uart_cfg={
    .baud_rate=115200,
    .data_bits=UART_DATA_8_BITS,
    .parity=UART_PARITY_DISABLE,
    .stop_bits=UART_STOP_BITS_1,
    .flow_ctrl=UART_HW_FLOWCTRL_DISABLE,
    .source_clk=UART_SCLK_DEFAULT
};

void init_uart()
{
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 512, 2048, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_cfg));
}

const char* header="FRAME";
void app_main(void)
{
    if(init_cam()!=ESP_OK) return;
    init_uart();

    while(1)
    {
        camera_fb_t *fb=esp_camera_fb_get();
        if(!fb)
        {
            ESP_LOGW("CAM","Capture fail");
        }
        else
        {    
            uart_write_bytes(UART_NUM_0, header, strlen(header));
            vTaskDelay(pdMS_TO_TICKS(500));
            uart_write_bytes(UART_NUM_0, (const char*)fb->buf, fb->len);

            esp_err_t err=uart_wait_tx_done(UART_NUM_0, pdMS_TO_TICKS(5000));
            if(err!=ESP_OK)
            {
                ESP_LOGW("CAM", "uart_tx wait fail %s", esp_err_to_name(err));
            }
            else esp_camera_fb_return(fb);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
