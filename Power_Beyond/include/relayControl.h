#ifndef RELAY_CONTROL_h
#define RELAY_CONTROL_h

#include "Arduino.h"

class RelayControl{
    public:
        RelayControl(uint8_t killPin, uint8_t bfPin);
        void startTask();  // Start the parallel task
        void init();
        uint8_t controlState;
    private:
        static void taskHandler(void *param);  // Task handler
        void continuousLoop();  // Function to run in the background task
        uint8_t killPin;
        uint8_t bfPin;
        uint32_t startTime;
        uint32_t bfKillTime;
        uint32_t compressorKillTime;
};



#endif