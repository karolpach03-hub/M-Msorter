#include "header.h"

TaskHandle_t task_ctrl_handle;
QueueHandle_t task_ctrl_queue;
bool autoreset;

uart_config_t uart_cfg={
    .baud_rate=115200,
    .data_bits=UART_DATA_8_BITS,
    .parity=UART_PARITY_DISABLE,
    .stop_bits=UART_STOP_BITS_1,
    .flow_ctrl=UART_HW_FLOWCTRL_DISABLE,
    .source_clk=UART_SCLK_DEFAULT
};

void app_main(void)
{
    task_ctrl_queue=xQueueCreate(10, sizeof(uint8_t));
    if(task_ctrl_queue==NULL)
    {
        ESP_LOGE("Main", "Queue creation fail");
        return;
    }
    state=SMS_IDLE;

    init_button();
    init_servos();
    init_cam();

    //temporary
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 512, 2048, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_cfg));

    if(xTaskCreate(task_controler, "ctrl", 2048, NULL, 1, &task_ctrl_handle)!=pdPASS)
    {
        ESP_LOGE("Main", "Task creation fail");
        return;
    }
}

SM_states_t state;
uint8_t debug_servo_id=1, debug_angle;
uint8_rgb_t debug_px[15]={0};
uint8_t dix=0;

uint8_t target_drop;
uint8_rgb_t average_pixel;
void task_controler(void *args)
{
    uint8_t ntcode, sendval=CTRL_NTCODE_NEXT_STEP;
    while(xQueueReceive(task_ctrl_queue, &ntcode, portMAX_DELAY))
    {
        switch(ntcode)
        {
        case CTRL_NTCODE_SW_CLICK:
            if(state==SMS_IDLE) xQueueSend(task_ctrl_queue, &sendval, 0);//start
            break;

        case CTRL_NTCODE_SW_PRESS:
            autoreset=!autoreset;
            if(state==SMS_IDLE) xQueueSend(task_ctrl_queue, &sendval, 0);//start
            break;

        case CTRL_NTCODE_NEXT_STEP:
            switch(state)
            {
            case SMS_IDLE:
                state=SMS_AQUIRE;
                break;

            case SMS_AQUIRE:
                vTaskDelay(pdMS_TO_TICKS(1000));
                set_servo_position(&servos[0], 1);//move to under camera
                vTaskDelay(pdMS_TO_TICKS(1500));

                take_photo();
                average_pixel=average_from_ROI();
                target_drop=make_decision(average_pixel);
                if(target_drop==0) autoreset=false;

                set_servo_position(&servos[0], 2);//move to arm
                vTaskDelay(pdMS_TO_TICKS(1000));
                state=SMS_DROP;
                break;

            case SMS_DROP:
                vTaskDelay(pdMS_TO_TICKS(1000));
                set_servo_position(&servos[1], target_drop);//move to drop
                vTaskDelay(pdMS_TO_TICKS(200*target_drop));

                set_servo_position(&servos[2], 1);//drop
                vTaskDelay(pdMS_TO_TICKS(1000));

                state=SMS_RETURN;
                break;

            case SMS_RETURN:
                set_servo_position(&servos[1], 0);
                set_servo_position(&servos[2], 0);
                set_servo_position(&servos[0], 0);
                vTaskDelay(pdMS_TO_TICKS(200*target_drop));
                target_drop=0;
                if(autoreset) state=SMS_AQUIRE;
                else state=SMS_IDLE;
                break;
            
            default:
                break;
            }

            if(state!=SMS_IDLE || autoreset==true) xQueueSend(task_ctrl_queue, &sendval, 0);
        break;
        
        default: break;
        }
    }
}

        // case CTRL_NTCODE_SW_CLICK:
        //         debug_angle=servos[debug_servo_id].cur_pos_id;
        //         debug_angle=(debug_angle+1)%servos[debug_servo_id].pos_num;

        //         set_servo_position(&servos[debug_servo_id], debug_angle);

        //         ESP_LOGI("CTRL", "servo %d to position %d -> %0.1f", 
        //             debug_servo_id, debug_angle, servos[debug_servo_id].angles[debug_angle]);
        //     break;

        // case CTRL_NTCODE_SW_PRESS:
        //         debug_servo_id=(debug_servo_id+1)%SRV_NUM;
        //         ESP_LOGI("CTRL", "setting servo %d", debug_servo_id);
        //     break;

// take_photo();
            // mms_t color;
            // for(uint8_t i=0; i<dix; i++)
            // {
            //     ESP_LOGI("MAIN", "rgb[%d]: (%d, %d, %d)", i, debug_px[i].r, debug_px[i].g, debug_px[i].b);
            //     color=make_decision(debug_px[i]);
            //     switch(color)
            //     {
            //     case MMS_BLUE: ESP_LOGW("Result", "blue"); break;
            //     case MMS_RED: ESP_LOGW("Result", "red"); break;
            //     case MMS_BROWN: ESP_LOGW("Result", "brow"); break;
            //     case MMS_YELLOW: ESP_LOGW("Result", "yellow"); break;
            //     case MMS_GREEN: ESP_LOGW("Result", "green"); break;
            //     case MMS_ORANGE: ESP_LOGW("Result", "orange"); break;
            //     case MMS_UNKNOWN: ESP_LOGW("Result", "UNKNOW"); break;
            //     case MMS_NONE: ESP_LOGW("Result", "NONE"); break;
                
            //     default:
            //         break;
            //     }

            //     debug_px[i].r=0;
            //     debug_px[i].g=0;
            //     debug_px[i].b=0;
            // }
            // dix=0;
            // set_servo_position(&servos[0], 1);
            // vTaskDelay(pdMS_TO_TICKS(500));
            // take_photo();
            // debug_px[dix]=average_from_ROI();
            // dix++;

            // vTaskDelay(pdMS_TO_TICKS(500));
            // set_servo_position(&servos[0], 2);
            // vTaskDelay(pdMS_TO_TICKS(1000));
            // set_servo_position(&servos[0], 0);