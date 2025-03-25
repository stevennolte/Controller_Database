#ifndef messageStructs_h
#define messageStructs_h
#include "Arduino.h"

#define HEARBEAT_PGN 5
#define MY_PGN 149

struct ProgramData_t{
  uint8_t wifiConnected;
  uint8_t AOGconnected;
  uint8_t can1connected;
  uint8_t can2connected;
  byte ina219Connected;
  uint32_t udpTimer;
  uint32_t can1Timer;
  uint32_t can2Timer;
  byte version1;
  byte version2;
  byte version3;
  bool shutdown;
  byte programState;   //1 = startup, 2 = connecting ....
  byte LEDbrightness;
  char sketchConfig[64];
  uint8_t ips[4];
  uint16_t serverPort;
  byte serverAddress;
  float gpsSpeed;
  uint16_t steerAngleSetPoint;
  bool Autosteer_running;
  uint8_t steerSwitch;
};

struct __attribute__ ((packed)) Shutdown_t{
    uint8_t aogByte1 : 8;
    uint8_t aogByte2 : 8;
    uint8_t sourceAddress : 8;
    uint8_t PGN : 8;
    uint8_t length : 8;
    uint8_t myPGN : 8;
    uint8_t confirm : 8;
    uint8_t checksum : 8;
  };

  union Shutdown_u{
  Shutdown_t shutdown_t;
  uint8_t bytes[sizeof(Shutdown_t)];
  };
  
struct __attribute__ ((packed)) Heartbeat_t{
    uint8_t aogByte1 : 8;
    uint8_t aogByte2 : 8;
    uint8_t sourceAddress : 8;
    uint8_t PGN : 8;
    uint8_t length : 8;
    uint8_t myPGN : 8;
    uint8_t version1 : 8;
    uint8_t version2 : 8;
    uint8_t version3 : 8;
    uint8_t keyPowerState : 8;
    uint8_t pcPowerState : 8;
    uint8_t isConnectedAOG : 8;
    uint8_t isConnectedCAN1 : 8;
    uint8_t isConnectedCAN2 : 8;
    uint8_t isConnectedISOVehicle : 8;
    uint8_t isConnectedISOImp : 8;
    uint8_t isConnectedINA219 : 8;
    uint8_t checksum : 8;
  };

  union Heartbeat_u{
  Heartbeat_t heartbeat_t;
  uint8_t bytes[sizeof(Heartbeat_t)];
  };

//Variables for settings
struct Storage {
  uint8_t Kp = 40;              // proportional gain
  uint8_t lowPWM = 10;          // band of no action
  int16_t wasOffset = 0;
  uint8_t minPWM = 9;
  uint8_t highPWM = 60;         // max PWM value
  float steerSensorCounts = 30;
  float AckermanFix = 1;        // sent as percent
};  Storage steerSettings;      // 11 bytes

//Variables for settings - 0 is false
struct Setup {
  uint8_t InvertWAS = 0;
  uint8_t IsRelayActiveHigh = 0;    // if zero, active low (default)
  uint8_t MotorDriveDirection = 0;
  uint8_t SingleInputWAS = 1;
  uint8_t CytronDriver = 1;
  uint8_t SteerSwitch = 0;          // 1 if switch selected
  uint8_t SteerButton = 0;          // 1 if button selected
  uint8_t ShaftEncoder = 0;
  uint8_t PressureSensor = 0;
  uint8_t CurrentSensor = 0;
  uint8_t PulseCountMax = 5;
  uint8_t IsDanfoss = 0;
  uint8_t IsUseY_Axis = 0;     //Set to 0 to use X Axis, 1 to use Y avis
}; Setup steerConfig;               // 9 bytes

#endif