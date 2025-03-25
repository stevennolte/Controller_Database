#ifndef PULSECTRL_h
#define PULSECTRL_h
#include "Arduino.h"
#include "driver/Ledc.h"

#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_CHANNEL            LEDC_CHANNEL_0


class PulseCtrl{
  public:
    PulseCtrl();
    void init(uint8_t _pin, ledc_channel_t _channel);
    void loop();

    uint16_t freqCmd = 60;
    uint16_t dutyCmd = 2000;
    uint32_t setDuty = 0;

  private:
    uint8_t pin;
    ledc_channel_t channel;
};  

#endif