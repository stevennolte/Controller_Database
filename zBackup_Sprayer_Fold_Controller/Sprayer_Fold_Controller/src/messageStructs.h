#ifndef messageStructs_h
#define messageStructs_h
#include "Arduino.h"

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
  } shutdown;
  

#endif