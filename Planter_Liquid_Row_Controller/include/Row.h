#ifndef Row_h
#define Row_h
#include "driver/Ledc.h"
#include "messageStructs.h"
#include "flowmeter.h"
#include "canFM.h"

class Row{
    public:
        Row();
        void init(byte flowPin, byte valvePin, ledc_channel_t channel);
        void updateValve();
        
        void startTask();
        uint64_t loopTime = 0;
        uint16_t highTime = 500;  //microseconds
        
        uint8_t sectionNum = 255;
        uint8_t sectionState = 0;
        uint16_t dutyCycleCMD = 0;
        uint16_t frequencyCMD = 60;
        uint64_t previousInterruptTime = 0;
        uint64_t currentInterruptTime = 0;
        bool onOffControl = false;

        FlowMeter flowmeter;
        CanFM canFM;
    private:
        uint64_t previousAvgTime = 0;
        byte valvePin;
        byte flowPin;
        uint16_t previousDutyCycle = 0;
        uint16_t previousFrequency = 0;
        ledc_channel_t channel;
        byte resolution = 6;
        
        static void taskHandler(void *param);
        void continuousLoop();
        
};

#endif