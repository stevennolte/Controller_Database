
## POWER_BEYOND
- The Power_Beyond module monitors the lift circuit and triggers the kickout of the bulkfill fan and the compressor.
- The trigger also runs a relay for 5 seconds to disable the bulkfill fan via the switch at the rear of the planter
- This module also acts as the main access point for the Wifi comms 
    - SSIE: "Power_Beyond"
    - password: "password"
    - IP: 192.168.0.75
    ### Control Logic
    - pressMon monitors the pressure sensors and detects a flank condition
    - this sets the following:
        - kill variable to high
        - starts the kill timer (7 seconds)
    - once the elasped time is up, the kill variable is set to low turning back on the fan and the compressor


    ### The Module also has it's own webserver for debuging, file upload, datalogging, firmware updates.

