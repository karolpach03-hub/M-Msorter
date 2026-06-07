#include "header.h"
//IODs - input/output devices

servo_t servos[SRV_NUM];
servo_config_t srv_cfg={
        .max_angle=SRV_MAX_ANGLE_DEG,
        .min_width_us=SRV_MIN_PULSEW_US,
        .max_width_us=SRV_MAX_PULSEW_US,
        .freq=SRV_FREQ_HZ,
        .timer_number=LEDC_TIMER_0,
        .channels={
            .servo_pin={
                SRV0_PIN,
                SRV1_PIN,
                SRV2_PIN
            },
            .ch={
                LEDC_CHANNEL_0,
                LEDC_CHANNEL_1,
                LEDC_CHANNEL_2
            }
        },
        .channel_number=SRV_NUM
    };

float srv0_angles[]={160, 95, 55};//disk/rotor - offset 9.64
float srv1_angles[]={175, 155, 130, 100, 75, 45, 15};//arm
float srv2_angles[]={5, 45};//spoon - offset 5

button_t sw={0};
servo_t init_servo(uint8_t servo_id);

//callback function for press/click of the button
void sw_cb(button_t *btn, button_state_t state)
{
    uint8_t sendval=0x00;
    if(state==BUTTON_CLICKED) sendval=CTRL_NTCODE_SW_CLICK;
    else 
    {
        if(state==BUTTON_PRESSED_LONG) sendval=CTRL_NTCODE_SW_PRESS;
        else return;
    }
    xQueueSend(task_ctrl_queue, &sendval, 0);
}

void init_servos()
{
    
    ESP_ERROR_CHECK(iot_servo_init(LEDC_LOW_SPEED_MODE, &srv_cfg));
    vTaskDelay(pdMS_TO_TICKS(200));


    for(uint8_t i=0; i<SRV_NUM; i++)
    {
        servos[i]=init_servo(i);
        ESP_ERROR_CHECK(iot_servo_write_angle(
            LEDC_LOW_SPEED_MODE, i, servos[i].angles[0]));
    }
}

servo_t init_servo(uint8_t servo_id)
{
    servo_t temp;
    temp.cur_pos_id=0;
    temp.servo_id=servo_id;

    switch(servo_id)
    {
    case 0:
        temp.pos_num=SRV0_POS_NUM;
        temp.angles=srv0_angles;
        break;

    case 1:
        temp.pos_num=SRV1_POS_NUM;
        temp.angles=srv1_angles;
        break;
    
    case 2:
        temp.pos_num=SRV2_POS_NUM;
        temp.angles=srv2_angles;
        break;
    
    default:
        ESP_LOGE("SRV", "Servo id=%d out of bounds", servo_id);
        exit(-1);
        break;
    }
    return temp;
}

void set_servo_position(servo_t *servo, uint8_t position)
{
    if(servo==NULL || position>=servo->pos_num) return;

    ESP_ERROR_CHECK(iot_servo_write_angle(
        LEDC_LOW_SPEED_MODE, servo->servo_id, servo->angles[position]));
    
    servo->cur_pos_id=position;
}

void init_button()
{
    sw.gpio=SW_PIN;
    sw.pressed_level=0;
    sw.internal_pull=true;
    sw.autorepeat=false;
    sw.callback=sw_cb;
    sw.ctx=NULL;
    ESP_ERROR_CHECK(button_init(&sw));
}
