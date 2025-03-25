#ifndef canFM_h
#define canFM_h
#include "Arduino.h"

class CanFM{
  public:
    uint16_t serialNum;
    uint64_t msgTimer;
    uint8_t sourceAddress;
    float flowrate;
    uint32_t flowcnt;
    uint8_t meterState;
    CanFM();
};

#endif