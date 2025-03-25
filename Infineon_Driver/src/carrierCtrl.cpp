#include "carrierCtrl.h"
#define frequencyFilter 0.1


CarrierCtrl::CarrierCtrl(){}

void CarrierCtrl::init(uint8_t _pin, ledc_channel_t _channel){
    this->pin = _pin;
    this->channel = _channel;
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        // .duty_resolution  = LEDC_DUTY_RES,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = freqCmd,  
        .clk_cfg          = LEDC_USE_APB_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .gpio_num       = pin,
        .speed_mode     = LEDC_MODE,
        .channel        = channel,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER,
        
        
        .duty           = dutyCmd, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void CarrierCtrl::loop(){
    
    if (highTrip){
        // vTaskDelay(5);
        dutyCmd = 5000;
        highTrip = false;
        vTaskDelay(10);
        dutyCmd = 8190;
    }
    // if (lowTrip){
    //     dutyCmd = 8191;
    // }
    if (dutyCmd != ledc_get_duty(LEDC_MODE, LEDC_CHANNEL)){
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, dutyCmd);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    }
    if (freqCmd > 10){
        if (freqCmd != ledc_get_freq(LEDC_MODE, LEDC_TIMER)){
            ledc_set_freq(LEDC_MODE, LEDC_TIMER, freqCmd);
        }
    }
}

