#include "Arduino.h"
#include "pressureMonitor.h"



PressureMonitor::PressureMonitor(uint8_t liftPin, uint8_t lowerPin)  {
  this->liftPin = liftPin;
  this->lowerPin = lowerPin;
  
}

void PressureMonitor::init(){
  pinMode(liftPin, INPUT);
  pinMode(lowerPin, INPUT);
  startTask();
}

// Task handler, runs in a separate task
void PressureMonitor::taskHandler(void *param) {
    // Cast the param back to the ClassA object
    PressureMonitor *instance = static_cast<PressureMonitor *>(param);
    instance->continuousLoop();  // Call the member function
}

// Start the FreeRTOS task
void PressureMonitor::startTask() {
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
void PressureMonitor::continuousLoop() {
  while (true) {
    pressureA = analogReadMilliVolts(liftPin);
    pressureB = analogReadMilliVolts(lowerPin);
    if (pressureA - pressureAprev > tripAthreshold){
      controlState = 1;
      vTaskDelay(1000/ portTICK_PERIOD_MS);
    } else {
      controlState = 0;
    }
    pressureAprev = pressureA;
    pressureBprev = pressureB;
    vTaskDelay(50 / portTICK_PERIOD_MS);  // Delay for 500ms to control the speed of the loop
  }
}
