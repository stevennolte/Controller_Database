#include "flowmeter.h"
#define frequencyFilter 0.95

FlowMeter::FlowMeter(){}

void FlowMeter::init(uint8_t _flowPin){
    flowPin = _flowPin;
}

void FlowMeter::loop(){
    if (esp_timer_get_time()-previousTimestamp > 500000 || cnt > 5){
    frequency = frequency*frequencyFilter + ((float)(cnt*1000000))/((float)(esp_timer_get_time() - previousTimestamp))*(1.0-frequencyFilter);
    cnt = 0;
    previousTimestamp = esp_timer_get_time();
    }
}

// class FlowMeter{
//   private:
//     uint8_t flowPin =0;
//   public:
//     volatile int cnt;
//     float gpmTarget;
//     float gpmRpt;
//     float frequency; 
//     uint64_t previousTimestamp;
//     uint64_t currentTimestamp;
//     uint16_t mVavg;
//     bool signalHigh;
//     FlowMeter(){
    
//     }
//     void init(uint8_t _flowPin){
      
//       flowPin = _flowPin;
      
      
//       // pinMode(flowPin, INPUT);
//     }
//     void loop(){
//       if (esp_timer_get_time()-previousTimestamp > 500000 || cnt > 5){
//         frequency = frequency*frequencyFilter + ((float)(cnt*1000000))/((float)(esp_timer_get_time() - previousTimestamp))*(1.0-frequencyFilter);
//         cnt = 0;
//         previousTimestamp = esp_timer_get_time();
//       }
//     }
// };
