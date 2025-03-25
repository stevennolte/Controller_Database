#ifndef messageStructs_h
#define messageStructs_h
#include "Arduino.h"



struct __attribute__ ((packed)) Log_t{
    uint64_t timestamp : 64;
    uint32_t analogReading : 32;
    
  };

  union Log_u{
  Log_t log_t;
  uint8_t bytes[sizeof(Log_t)];
  };

  #endif