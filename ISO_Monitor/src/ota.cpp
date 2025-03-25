#include "Arduino.h"
#include "ota.h"

void OtaPullCallback(int offset, int totallength)
{
	Serial.printf("Updating %d of %d (%02d%%)...\r", offset, totallength, 100 * offset / totallength);
// #if defined(LED_BUILTIN) // flicker LED on update
// 	static int status = LOW;
// 	status = status == LOW && offset < totallength ? HIGH : LOW;
// 	digitalWrite(LED_BUILTIN, status);
// #endif
}