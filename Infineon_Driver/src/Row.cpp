#include "Row.h"
#include "Arduino.h"


// #define LEDC_MODE               LEDC_LOW_SPEED_MODE
// #define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
// #define LEDC_TIMER              LEDC_TIMER_0
// #define LEDC_CHANNEL_00            LEDC_CHANNEL_0
// #define LEDC_CHANNEL_01           LEDC_CHANNEL_1
// #define LEDC_CHANNEL_02            LEDC_CHANNEL_2
// #define LEDC_CHANNEL_03            LEDC_CHANNEL_3


Row::Row(){
    FlowMeter flowmeter = FlowMeter();
    CanFM canFM = CanFM();
}

// void Row::init(byte pulsePin, byte carrierPin, byte dirPin, byte pwmInputPin, byte valvePin, ledc_channel_t pulseCarrier, ledc_channel_t carrerChannel){
void Row::init(){
    this ->pulsePin = 17;
    this->carrierPin = 10;
    this ->flowPin = flowPin;
    this ->carrierChannel = LEDC_CHANNEL_1;
    this->pulseChannel = LEDC_CHANNEL_0;
    this->dirPin = 11;
    this->pwmInputPin = 18;
    flowmeter.init(flowPin);
    
    pulse.init(pulsePin,pulseChannel);
    carrier.init(carrierPin,carrierChannel);
    pinMode(dirPin, OUTPUT);
    digitalWrite(dirPin, LOW);
    startTask();
    
}

void Row::taskHandler(void *param) {
    // Cast the param back to the ClassA object
    Row *instance = static_cast<Row *>(param);
    instance->continuousLoop();  // Call the member function
}

void Row::startTask() {
    xTaskCreate(
        taskHandler,   // Task function
        "rowTask",       // Name of the task
        4096,          // Stack size (in words)
        this,          // Pass the current instance as the task parameter
        0,             // Priority of the task
        NULL           // Task handle (not needed)
    );
}

void Row::continuousLoop() {
  while (true) {
    flowmeter.loop();
    if (millis()-watchdogTimer > 1000){
        pulse.dutyCmd = 0;
    }
    pulse.loop();
    carrier.loop();
    vTaskDelay(1);
    }

}