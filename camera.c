#include "header.h"

camera_fb_t *fb;

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
    .xclk_freq_hz=10000000,
    .ledc_timer=LEDC_TIMER_1,
    .ledc_channel=LEDC_CHANNEL_7,
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

    fb=NULL;
    return ESP_OK;
}

uint8_rgb_t average_from_ROI()
{
    uint8_rgb_t px={0};
    if(fb!=NULL)
    {
        uint16_t pixel;
        uint16_t index;
        uint16_t sum_r=0, sum_g=0, sum_b=0;
        uint16_t row_max=SEN_PX_ROW_START+SEN_PX_ROW_NUM*SEN_PX_ROW_INTRVL;
        uint16_t col_max=SEN_PX_COL_START+SEN_PX_COL_NUM*SEN_PX_COL_INTRVL;
        
        for(uint16_t row=SEN_PX_ROW_START; row<row_max; row+=SEN_PX_ROW_INTRVL)
        {
            for(uint16_t col=SEN_PX_COL_START; col<col_max; col+=SEN_PX_COL_INTRVL)
            {
                index=(row*SEN_FRAME_WIDTH+col)*2;
                pixel=(fb->buf[index]<<8)|(fb->buf[index+1]);
                px.r=(pixel>>11)<<3;
                px.g=((pixel>>5)&0x3f)<<2;
                px.b=(pixel&0x1F)<<3;

                sum_r+=px.r;
                sum_g+=px.g;
                sum_b+=px.b;
            }
        }
        px.r=sum_r/(SEN_PX_ROW_NUM*SEN_PX_COL_NUM);
        px.g=sum_g/(SEN_PX_ROW_NUM*SEN_PX_COL_NUM);
        px.b=sum_b/(SEN_PX_ROW_NUM*SEN_PX_COL_NUM);
    }
    return px;
}
const mms_profile_t mms_lib[]=
{
    {MMS_NONE, 335, 358},
    {MMS_BLUE, 315, 348},
    {MMS_BROWN, 340, 350},
    {MMS_RED, 357, 336},
    {MMS_YELLOW, 348, 365},
    {MMS_GREEN, 325, 365},
    {MMS_ORANGE, 357, 348},
};
uint8_t make_decision(uint8_rgb_t av_color)
{
    uint32_t total=av_color.r+av_color.g+av_color.b;
    if(total<100) return MMS_NONE;
    uint32_rgb_t ratio;

    ratio.r=((uint32_t)av_color.r*1000)/total;
    ratio.g=((uint32_t)av_color.g*1000)/total;

    uint32_t min_dist=UINT32_MAX, dist=0;
    int32_t delta_r=0, delta_g=0;
    uint8_t best_match=0;
    for(uint8_t i=0; i<7; i++)
    {
        delta_r=(int32_t)ratio.r-(int32_t)mms_lib[i].r_target;
        delta_g=(int32_t)ratio.g-(int32_t)mms_lib[i].g_target;
        dist=delta_r*delta_r+delta_g*delta_g;
        if(dist<min_dist)
        {
            min_dist=dist;
            best_match=i;
        }
    }
    return best_match;
}

void take_photo()
{
    if(fb!=NULL)
    {
        esp_camera_fb_return(fb);
    }
    fb=esp_camera_fb_get();
    //ESP_LOGI("CAM", "taken photo");
}

void send_photo()
{
    if(fb==NULL) 
    {
        ESP_LOGW("CAM", "fb is null");
        return;
    }

    uart_write_bytes(UART_NUM_0, "FRAME", strlen("FRAME"));
    vTaskDelay(pdMS_TO_TICKS(500));
    uart_write_bytes(UART_NUM_0, (const char*)fb->buf, fb->len);

    esp_err_t err=uart_wait_tx_done(UART_NUM_0, pdMS_TO_TICKS(5000));
    if(err!=ESP_OK)
    {
        ESP_LOGW("CAM", "uart_tx wait fail %s", esp_err_to_name(err));
    }
}