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

uint8_t target_drop;
uint8_rgb_t average_pixel;

#if DEBUG_REPLACE_AUTO_WITH_LOG
debug_info_t debug_info[DEBUG_LOG_LEN];
uint8_t debug_idx;
#endif

//main task
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
            #if DEBUG_REPLACE_AUTO_WITH_LOG
                if(state==SMS_IDLE)
                {
                    ESP_LOGW("LOG", "printing %d samples", debug_idx);
                    for(uint8_t i=0; i<debug_idx; i++)
                    {
                        printf("%d; %d; %d; %d\n", 
                        debug_info[i].avg_px.r,
                        debug_info[i].avg_px.g,
                        debug_info[i].avg_px.b,
                        debug_info[i].type);
                    }
                    debug_idx=0;
                }
            #else
                //normal operation
                autoreset=!autoreset;
                if(state==SMS_IDLE) xQueueSend(task_ctrl_queue, &sendval, 0);//start
            #endif
            break;

        case CTRL_NTCODE_NEXT_STEP:
            switch(state)
            {
            case SMS_IDLE:
                state=SMS_AQUIRE;
                break;

            case SMS_AQUIRE://first step - rotor
                vTaskDelay(pdMS_TO_TICKS(1000));
                set_servo_position(&servos[0], 1);//move to under camera
                vTaskDelay(pdMS_TO_TICKS(2000));

                take_photo();
                average_pixel=average_from_ROI();
                target_drop=make_decision(average_pixel);
                if(target_drop==0)//none detected
                {   
                    //autoreset=false;
                    //state=SMS_AQUIRE;
                }
                #if DEBUG_REPLACE_AUTO_WITH_LOG
                    debug_info[debug_idx].avg_px=average_pixel;
                    debug_info[debug_idx].type=target_drop;
                    debug_idx=(debug_idx+1)%DEBUG_LOG_LEN;
                #endif

                set_servo_position(&servos[0], 2);//move to arm
                vTaskDelay(pdMS_TO_TICKS(1000));
                state=SMS_DROP;
                break;

            case SMS_DROP://second step - arm
                vTaskDelay(pdMS_TO_TICKS(1000));
                set_servo_position(&servos[1], target_drop);//move to drop position
                vTaskDelay(pdMS_TO_TICKS(200*target_drop));

                set_servo_position(&servos[2], 1);//drop to trough
                vTaskDelay(pdMS_TO_TICKS(1000));

                state=SMS_RETURN;
                break;

            case SMS_RETURN://reset all servos
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