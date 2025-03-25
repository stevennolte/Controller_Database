#ifndef PRESSURE_MONITOR_h
#define PRESSURE_MONITOR_h

#include <Adafruit_NeoPixel.h>

class PressureMonitor{
    public:
        PressureMonitor(uint8_t liftPin, uint8_t lowerPin);
        void startTask();  // Start the parallel task
        void init();
        uint8_t controlState;
        uint16_t pressureA;
        uint16_t pressureB;
    private:
        static void taskHandler(void *param);  // Task handler
        void continuousLoop();  // Function to run in the background task
        uint8_t liftPin;
        uint8_t lowerPin;
        uint16_t tripAthreshold = 500;
        uint16_t tripBthreshold = 500;
        uint16_t pressureAprev = 0;
        uint16_t pressureBprev = 0;
};



#endif