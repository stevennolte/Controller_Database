#ifndef CARRIERCTRL_h
#define CARRIERCTRL_h
#include "Arduino.h"
#include "driver/ledc.h"

#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_TIMER              LEDC_TIMER_2
#define LEDC_CHANNEL            LEDC_CHANNEL_1



class CarrierCtrl{
  public:
    void init(uint8_t _pin, ledc_channel_t _channel);
    void loop();
    void onTimer();
    CarrierCtrl();
    uint16_t freqCmd = 2500;
    uint16_t dutyCmd = 8190;
    bool highTrip;
    bool lowTrip;

  private:
    uint8_t pin;
    ledc_channel_t channel;
    
};

#endif