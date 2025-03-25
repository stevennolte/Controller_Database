#include "Arduino.h"
#include "relayControl.h"



RelayControl::RelayControl(uint8_t killPin, uint8_t bfPin)  {
  this->killPin = killPin;
  this->bfPin = bfPin;
  
}

void RelayControl::init(){
  pinMode(killPin, OUTPUT);
  pinMode(bfPin, OUTPUT);
  bfKillTime = 3000;
  compressorKillTime = 7000;
  
  startTask();
}

// Task handler, runs in a separate task
void RelayControl::taskHandler(void *param) {
    // Cast the param back to the ClassA object
    RelayControl *instance = static_cast<RelayControl *>(param);
    instance->continuousLoop();  // Call the member function
}

// Start the FreeRTOS task
void RelayControl::startTask() {
    xTaskCreate(
        taskHandler,   // Task function
        "TaskA",       // Name of the task
        4096,          // Stack size (in words)
        this,          // Pass the current instance as the task parameter
        1,             // Priority of the task
        NULL           // Task handle (not needed)
    );
}

// Function to run in parallel
void RelayControl::continuousLoop() {
  while (true) {
    if (controlState == 1){
      startTime = millis();
      digitalWrite(killPin, HIGH);
      digitalWrite(bfPin, HIGH);
      while (millis()- startTime < bfKillTime){
        vTaskDelay(10/portTICK_PERIOD_MS);
      }
      digitalWrite(bfPin, LOW);
      while (millis()- startTime < compressorKillTime){
        vTaskDelay(10/portTICK_PERIOD_MS);
      }
      digitalWrite(bfPin, HIGH);
      digitalWrite(killPin, LOW);
       while (millis()- startTime < bfKillTime){
        vTaskDelay(10/portTICK_PERIOD_MS);
      }
      digitalWrite(bfPin, LOW);
      controlState = 0;

    }
    vTaskDelay(10 / portTICK_PERIOD_MS);  // Delay for 500ms to control the speed of the loop
  }
}
