#ifndef MyWifi_h
#define MyWifi_h

#include "Arduino.h"


class MyWifi
{
    private:
        
        
        const char* ssids[4] = {"NOLTE_FARM", "FERT"};
        const char* passwords[4] = {"DontLoseMoney89","Fert504!"};
    public:
        const char* ssid = "";
        const char* password = "";
        MyWifi();
        uint8_t connect(uint8_t * ipAddr);
        IPAddress gateway();
        IPAddress subnet();
        IPAddress local_IP();
        // uint8_t ipAddress[4];
        // uint8_t address_1;
        // uint8_t address_2;
        // uint8_t address_3;
        // uint8_t address_4; 
       
};
#endif