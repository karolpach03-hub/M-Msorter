/*main header contaning macros (settings) and global variable/function declarations*/

#pragma once
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_camera.h"
#include "driver/uart.h"
#include "button.h"
#include <iot_servo.h>

#define DEBUG_REPLACE_AUTO_WITH_LOG     false

/*  
====================================================================
        Camera
====================================================================    
*/
esp_err_t init_cam();

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

#define SEN_FRAME_WIDTH     128
#define SEN_FRAME_HEIGHT    128

#define SEN_PX_ROW_START    16
#define SEN_PX_COL_START    31
#define SEN_PX_ROW_NUM      11
#define SEN_PX_COL_NUM      11
#define SEN_PX_ROW_INTRVL   4
#define SEN_PX_COL_INTRVL   4

extern uart_config_t uart_cfg;

typedef struct
{
    uint8_t r, g, b;
}uint8_rgb_t;

typedef struct
{
    uint16_t r, g, b;
}uint16_rgb_t;

typedef struct
{
    uint32_t r, g, b;
}uint32_rgb_t;

typedef enum{
    MMS_UNKNOWN, MMS_RED, MMS_BLUE, MMS_BROWN, MMS_ORANGE, MMS_YELLOW, MMS_GREEN, MMS_NONE
}mms_t;

typedef struct{
    mms_t type;
    uint32_t r_target;
    uint32_t g_target;
}mms_profile_t;

#if DEBUG_REPLACE_AUTO_WITH_LOG
typedef struct{
    uint8_rgb_t avg_px;
    uint8_t type;
}debug_info_t;
#define DEBUG_LOG_LEN   32
extern debug_info_t debug_info[DEBUG_LOG_LEN];
extern uint8_t debug_idx;
#endif

extern const mms_profile_t mms_lib[];

void take_photo();
void send_photo();
uint8_rgb_t average_from_ROI();
uint8_t make_decision(uint8_rgb_t av_color);

/*  
====================================================================
    IODs - input/output devices
====================================================================    
*/
#define SRV0_PIN        33
#define SRV0_POS_NUM    3
extern float srv0_angles[];

#define SRV1_PIN        14
#define SRV1_POS_NUM    7
extern float srv1_angles[];
#define SRV2_PIN        12
#define SRV2_POS_NUM    2
extern float srv2_angles[];

#define SRV_NUM             3
#define SRV_MAX_ANGLE_DEG   180
#define SRV_MAX_PULSEW_US   2500
#define SRV_MIN_PULSEW_US   500
#define SRV_FREQ_HZ         50

typedef struct
{
    float* angles;
    uint8_t cur_pos_id;
    uint8_t pos_num;
    uint8_t servo_id;
}servo_t;

extern servo_t servos[SRV_NUM];

void set_servo_position(servo_t *servo, uint8_t position);
void init_servos();

#define SW_PIN          13   
void init_button();

/*  
====================================================================
    Controller
====================================================================    
*/

extern TaskHandle_t task_ctrl_handle;
extern QueueHandle_t task_ctrl_queue;

extern bool autoreset;

void task_controler(void *args);
#define CTRL_NTCODE_SW_CLICK    0
#define CTRL_NTCODE_SW_PRESS    1
#define CTRL_NTCODE_NEXT_STEP   2

typedef enum{
    SMS_IDLE,
    SMS_AQUIRE,
    SMS_DROP,
    SMS_RETURN,
}SM_states_t;

extern SM_states_t state;