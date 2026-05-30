#include "header.h"

TaskHandle_t task_ctrl_handle;

void app_main(void)
{
    //if(init_cam()!=ESP_OK) return;
    init_button();
    init_servos();

    if(xTaskCreate(task_controler, "ctrl", 2048, NULL, 1, &task_ctrl_handle)!=pdPASS)
    {
        ESP_LOGE("Main", "Task creation fail");
        return;
    }
    vTaskDelay(portMAX_DELAY);
}

uint8_t debug_servo_id=1, debug_angle;
void task_controler(void *args)
{
    uint32_t ntcode;
    while(xTaskNotifyWait(0x00, 0xff, &ntcode, portMAX_DELAY))
    {
        switch(ntcode)
        {
        case CTRL_NTCODE_SW_CLICK:
                debug_angle=servos[debug_servo_id].cur_pos_id;
                debug_angle=(debug_angle+1)%servos[debug_servo_id].pos_num;

                set_servo_position(&servos[debug_servo_id], debug_angle);

                ESP_LOGI("CTRL", "servo %d to position %d -> %0.1f", 
                    debug_servo_id, debug_angle, servos[debug_servo_id].angles[debug_angle]);
            break;

        case CTRL_NTCODE_SW_PRESS:
                debug_servo_id=(debug_servo_id+1)%SRV_NUM;
                ESP_LOGI("CTRL", "setting servo %d", debug_servo_id);
            break;
        
        default:
            break;
        }
    }
}

uart_config_t uart_cfg={
    .baud_rate=115200,
    .data_bits=UART_DATA_8_BITS,
    .parity=UART_PARITY_DISABLE,
    .stop_bits=UART_STOP_BITS_1,
    .flow_ctrl=UART_HW_FLOWCTRL_DISABLE,
    .source_clk=UART_SCLK_DEFAULT
};


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
