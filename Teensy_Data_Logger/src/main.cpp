#include <Arduino.h>
#include <SD.h>
#include <ADC.h>
#include <HardwareSerial.h>

#define ARRAY_SIZE 5000
uint32_t stoptime = 30000;
uint16_t dataArray[ARRAY_SIZE];

struct __attribute__ ((packed)) Log_t{
    uint16_t averageValue : 16;
    uint16_t minValue : 16;
    uint16_t maxValue : 16;
    
  };

  union Log_u{
  Log_t log_t;
  uint8_t bytes[sizeof(Log_t)];
  } logData;

// Pin definitions
const int analogPin = 15; // GPIO14 is A1 (analog pin 14)

// ADC object
ADC *adc = new ADC();


#define BAUD_RATE 115200
HardwareSerial &uart = Serial2; // Use Serial1 for UART

// // Buffer settings
// const size_t bufferSize = 1024; // Size of the buffer
// char logBuffer[bufferSize * 32]; // Allocate enough space for log entries
// size_t bufferIndex = 0; // Current index in the buffer
void sendUint16(uint16_t value) {
    uint8_t highByte = (value >> 8) & 0xFF; // Extract the high byte
    uint8_t lowByte = value & 0xFF;         // Extract the low byte

    uart.write(highByte); // Send the high byte
    uart.write(lowByte);  // Send the low byte
    Serial.println("Sent Data: " + String(value));
}

void setup() {
  // Initialize serial
  Serial.begin(BAUD_RATE);
  uart.begin(BAUD_RATE);
  delay(1000);
  // Initialize ADC
  adc->adc0->setAveraging(8); // Average multiple readings for stability
  adc->adc0->setResolution(16); // 10-bit resolution
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED); 
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED); 

  Serial.println("Setup complete.");
}



void loop() {
  // sendUint16(1000);
  // Serial.println("Sampling Pin");
  for (uint16_t i=0;i<ARRAY_SIZE;i++){
    // Serial.println("Sampling Pin " + String(i));
    uint16_t analogValue = adc->adc0->analogRead(analogPin);
    dataArray[i] = analogValue;
  }
  Serial.println(adc->adc0->analogRead(analogPin));
  
  uint64_t valueSum = 0;
  uint16_t minVal = 0;
  uint16_t maxVal = 0;
  for (uint16_t i=0;i<ARRAY_SIZE;i++){
    if (dataArray[i] > maxVal){
      maxVal = dataArray[i];
    }
    if (minVal == 0){
      minVal = dataArray[i];
    }
    if (dataArray[i] < minVal){
      minVal = dataArray[i];
    }
    valueSum = valueSum + dataArray[i];
  }

  uint16_t averageValue = valueSum/ARRAY_SIZE;
  logData.log_t.averageValue = averageValue;
  logData.log_t.minValue = minVal;
  logData.log_t.maxValue = maxVal;
  uint8_t _buffer[6];
  // _buffer[0] = averageValue & 0xFF;
  // _buffer[1] = (averageValue >> 8) & 0xFF;

  // _buffer[2] = minVal & 0xFF;
  // _buffer[3] = (minVal >> 8) & 0xFF;

  // _buffer[4] = maxVal & 0xFF;
  // _buffer[5] = (maxVal >> 8) & 0xFF;
  // _buffer[0] = 1;
  // _buffer[1] = 2;

  // _buffer[2] = 3;
  // _buffer[3] = 4;

  // _buffer[4] = 5;
  // _buffer[5] = 6;
  for (int i=0; i<sizeof(_buffer);i++){
    _buffer[i] = logData.bytes[i];
    // Serial.print(_buffer[i]);
    // Serial.print(" ");
  }
  // Serial.println();
  uart.write(_buffer, sizeof(_buffer));
  uart.flush();
  
  // Serial.println();
  Serial.print(String(logData.log_t.averageValue));
  Serial.print(" ");
  Serial.print(String(logData.log_t.maxValue));
  Serial.print(" ");
  Serial.println(String(logData.log_t.minValue));
  delay(10);
}
