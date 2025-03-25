#include "Row.h"
#include "Arduino.h"


#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_14_BIT // Set duty resolution to 13 bits
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_CHANNEL_00            LEDC_CHANNEL_0
#define LEDC_CHANNEL_01           LEDC_CHANNEL_1
#define LEDC_CHANNEL_02            LEDC_CHANNEL_2
#define LEDC_CHANNEL_03            LEDC_CHANNEL_3


Row::Row(){
    FlowMeter flowmeter = FlowMeter();
    CanFM canFM = CanFM();
}

void Row::init(byte flowPin, byte valvePin, ledc_channel_t channel){
    this ->valvePin = valvePin;
    this ->flowPin = flowPin;
    this ->channel = channel;
    
    flowmeter.init(flowPin);
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        // .duty_resolution  = LEDC_DUTY_RES,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = frequencyCMD,  
        .clk_cfg          = LEDC_USE_APB_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .gpio_num       = valvePin,
        .speed_mode     = LEDC_MODE,
        .channel        = channel,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER,
        
        
        .duty           = dutyCycleCMD, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void Row::updateValve(){
      
        if (dutyCycleCMD != previousDutyCycle){
            ledc_set_duty(LEDC_MODE, channel, dutyCycleCMD);
            ledc_update_duty(LEDC_MODE, channel);
            previousDutyCycle = dutyCycleCMD;
        }

        if (frequencyCMD != previousFrequency){
            // Serial.print("Setting Frequency to: ");
            // Serial.println(frequencyCMD);
            if (frequencyCMD < 10){
            frequencyCMD = 10;
            }
            ledc_set_freq(LEDC_MODE, LEDC_TIMER, frequencyCMD);
            previousFrequency = frequencyCMD;
        }
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
        1,             // Priority of the task
        NULL           // Task handle (not needed)
    );
}

void Row::continuousLoop() {
  while (true) {
    
    
  }
}


// class Row{
//   private:
//     uint64_t previousAvgTime = 0;
//     byte valvePin;
//     byte flowPin;
//     uint16_t previousDutyCycle = 0;
//     uint16_t previousFrequency = 0;
//     ledc_channel_t channel;
//     byte resolution = 6;
//   public:
    
//     uint8_t sectionNum = 255;
//     uint8_t sectionState = 0;
//     uint16_t dutyCycleCMD = 0;
//     uint16_t frequencyCMD = 60;
//     uint64_t previousInterruptTime = 0;
//     uint64_t currentInterruptTime = 0;
//     FlowMeter flowmeter = FlowMeter();
//     CanFM canFM = CanFM();
//     Row(){
      
//     }

//     void init(byte flowPin, byte valvePin, ledc_channel_t channel)
//       {
//         this ->valvePin = valvePin;
//         this ->flowPin = flowPin;
//         this ->channel = channel;
        
//         flowmeter.init(flowPin);
//     // Prepare and then apply the LEDC PWM timer configuration
//           ledc_timer_config_t ledc_timer = {
//               .speed_mode       = LEDC_MODE,
//               // .duty_resolution  = LEDC_DUTY_RES,
//               .duty_resolution  = LEDC_DUTY_RES,
//               .timer_num        = LEDC_TIMER,
//               .freq_hz          = frequencyCMD,  // Set output frequency at 4 kHz
//               .clk_cfg          = LEDC_AUTO_CLK
//           };
//           ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

//           ledc_channel_config_t ledc_channel = {
//               .gpio_num       = valvePin,
//               .speed_mode     = LEDC_MODE,
//               .channel        = channel,
//               .intr_type      = LEDC_INTR_DISABLE,
//               .timer_sel      = LEDC_TIMER,
              
              
//               .duty           = dutyCycleCMD, // Set duty to 0%
//               .hpoint         = 0
//           };
//           ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
 
//       }

//     void updateValve(){
//       // ledc_set_freq(LEDC_MODE, LEDC_TIMER, valveData.frequencyCMD);
      
//       // if (dutyCycleCMD != ledcRead(channel)){
        
//         ledc_set_duty(LEDC_MODE, channel, dutyCycleCMD);
//         ledc_update_duty(LEDC_MODE, channel);
//         previousDutyCycle = dutyCycleCMD;
        
        
//       // }
//       if (frequencyCMD != ledc_get_freq(LEDC_MODE,LEDC_TIMER)){
//         // Serial.print("Setting Frequency to: ");
//         // Serial.println(frequencyCMD);
//         if (frequencyCMD == 0){
//           frequencyCMD = 10;
//         }
//         ledc_set_freq(LEDC_MODE, LEDC_TIMER, frequencyCMD);
        
//       }
//     }

    
// };