#include "header.h"


void app_main(void)
{
    //if(init_cam()!=ESP_OK) return;



    
}



// void init_uart()
// {
//     ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 512, 2048, 0, NULL, 0));
//     ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_cfg));
// }

// const char* header="FRAME";

// void aquire_color()
// {
//     camera_fb_t *fb=esp_camera_fb_get();
//     if(!fb)ESP_LOGW("CAM","Capture fail");
    
//     uint32_t average_color=0;
    
// }
// uint16_t get_px(uint8_t row, uint8_t col, camera_fb_t *fb)
// {
//     uint16_t *px=(uint16_t*)fb->buf;
//     return px[(row*fb->width+col)];
// }
// void constant_photos()
// {
//     init_uart();

//     while(1)
//     {
//         camera_fb_t *fb=esp_camera_fb_get();
//         if(!fb)
//         {
//             ESP_LOGW("CAM","Capture fail");
//         }
//         else
//         {    
//             uart_write_bytes(UART_NUM_0, header, strlen(header));
//             vTaskDelay(pdMS_TO_TICKS(500));
//             uart_write_bytes(UART_NUM_0, (const char*)fb->buf, fb->len);

//             esp_err_t err=uart_wait_tx_done(UART_NUM_0, pdMS_TO_TICKS(5000));
//             if(err!=ESP_OK)
//             {
//                 ESP_LOGW("CAM", "uart_tx wait fail %s", esp_err_to_name(err));
//             }
//             else esp_camera_fb_return(fb);
//         }
//         vTaskDelay(pdMS_TO_TICKS(5000));
//     }
// }
