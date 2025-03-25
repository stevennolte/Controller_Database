#ifndef flowmeter_h
#define flowmeter_h
#include "Arduino.h"

class FlowMeter{
  private:
    uint8_t flowPin =0;
  public:
    volatile int cnt;
    float gpmTarget;
    float gpmRpt;
    float frequency; 
    uint64_t previousTimestamp;
    uint64_t currentTimestamp;
    uint16_t mVavg;
    bool signalHigh;
    FlowMeter();
    void init(uint8_t _flowPin);
    
    void loop();
};


#endif