#ifndef Row_h
#define Row_h

#include "driver/Ledc.h"
#include "messageStructs.h"
#include "flowmeter.h"
#include "canFM.h"
#include "carrierCtrl.h"
#include "pulseCtrl.h"

class Row{
    public:
        Row();
        // void init(byte pulsePin, byte carrierPin, byte dirPin, byte pwmInputPin, byte valvePin, ledc_channel_t pulseCarrier, ledc_channel_t carrerChannel);
        void init();
        void updateValve();
        
        void startTask();
        uint8_t pwmCtrlState = 0;
        uint8_t sectionNum = 255;
        uint8_t sectionState = 0;
        uint32_t watchdogTimer = 0;
        uint8_t carrierPin;
        uint8_t pulsePin;
        uint8_t flowPin;
        uint8_t dirPin;
        uint8_t pwmInputPin;
        
        CarrierCtrl carrier;
        PulseCtrl pulse;
        FlowMeter flowmeter;
        CanFM canFM;
        struct __attribute__ ((packed)) CANOut_t{
            uint8_t row : 8;
            uint16_t frequency : 16;
            uint16_t flow : 16;
            
            // uint16_t placeholder : 24;
    
        };

        union CANOut_u{
        CANOut_t canOut_t;
        uint8_t bytes[sizeof(CANOut_t)];
        } canOut;
    private:
        uint64_t previousAvgTime = 0;
        
        uint16_t previousDutyCycle = 0;
        uint16_t previousFrequency = 0;
        ledc_channel_t carrierChannel;
        ledc_channel_t pulseChannel;
        byte resolution = 6;
        

        

        
        
        static void taskHandler(void *param);
        void continuousLoop();
        
};

#endif