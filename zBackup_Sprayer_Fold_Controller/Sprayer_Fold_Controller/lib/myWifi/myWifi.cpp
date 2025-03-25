#include <WiFi.h>
#include "Arduino.h"
#include "myWifi.h"



MyWifi::MyWifi()
{
    
};

uint8_t MyWifi::connect(uint8_t * ipAddr){
    // IPAddress local_IP(192, 168, 0, getAddress());
    IPAddress local_IP(ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
    IPAddress gateway(ipAddr[0], ipAddr[1], ipAddr[2], 1);
    IPAddress subnet(255, 255, 255, 0);
    
    int n = WiFi.scanNetworks();
    for (int i=0; i<min(n,25); i++){
        for (int si=0; si<3;si++){
          
          if (WiFi.SSID(i) == ssids[si]){
            ssid = ssids[si];
            password = passwords[si];
            Serial.print("\tWifi IP: ");
            Serial.println(ssid);
            
          }
        }  
    }
    if (ssid == ""){
        Serial.println("\tNo WiFi Found");
        return 2;
    }
    
    if (!WiFi.config(local_IP, gateway, subnet)) {
              Serial.println("STA Failed to configure");
    }
    // WiFi.mode(WIFI_AP);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        delay(1000);
    }
    Serial.print("\tIP Address: ");
    Serial.println(WiFi.localIP());
    
    return 1;
}