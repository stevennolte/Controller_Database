#include <Arduino.h>
#include <myWifi.h>

#include "ESP32OTAPull.h"
#include "Version.h"
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "AsyncUDP.h"
#include "messageStructs.h"
#include "myLED.h"
#include "configLoad.h"
#include "vector"
#include <driver/twai.h>
#include <driver/gpio.h>
#include "Row.h"


#define NUM_OF_ROWS 6
#define CAN1_RX_PIN 2
#define CAN1_TX_PIN 1
#define POWER_RELAY_PIN 14

const uint8_t sectionFlowPins[] = {4,5,6,7,15,16};
// const uint8_t adcChans[] = {3,4,5,6};
const uint8_t sectionValvePins[] = {13,12,11,10,9,8};

Heartbeat_u heartbeat;
Heartbeat_t * hbData = &heartbeat.heartbeat_t;
Shutdown_u shutdownMsg;

std::vector<String> debugVars;
ProgramData_t progData;
MyLED myLED(progData, 48, 25);
ConfigLoad config(progData);
MyWifi myWifi;
AsyncWebServer server(80);
ESP32OTAPull ota;
Row rows[NUM_OF_ROWS];



class UDPMethods{
  private:
    
    uint32_t udpTimeout = 1000;
    
    int heartbeatTimePrevious=0;
    int heartbeatTimeTrip=250;
    int flowDataTimePrevious=0;
    int flowDataTimeTrip=1000;
  public:
    AsyncUDP udp;
    
    UDPMethods(){
    }
    
    void begin(){
      udp.listen(8888);
      Serial.println("UDP Listening");
      
      char version[64];
      strcpy(version, VERSION);
      char *token = strtok(version, ".");
      int i = 0;
      while (token != NULL) {
        int intValue = atoi(token);
        switch (i){
          case 0:
            hbData->version1 = intValue;
            break;
          case 1:
            hbData->version2 = intValue;
            break;
          case 2:
            hbData->version3 = intValue;
            break;
        }
        i++;
        token = strtok(NULL, ".");
      }
      Serial.println("testbreak");
      
      hbData->aogByte1 = 0x80;
      hbData->aogByte2 = 0x81;
      hbData->sourceAddress = 51;
      hbData->PGN = MY_PGN;
      hbData->length = 10;
      hbData->myPGN = HEARBEAT_PGN;
      udp.onPacket([](AsyncUDPPacket packet) {
        if (packet.data()[0]==0x80 & packet.data()[1]==0x81){
          progData.udpTimer = millis();
          switch (packet.data()[3]){
            case 201:
              
              progData.ips[0] = packet.data()[7];
              progData.ips[1] = packet.data()[8];
              progData.ips[2] = packet.data()[9];
              // progData.updateConfig();
              ESP.restart();
              break;
          
            case 149:
              switch (packet.data()[5])
                {
                case 5:
                  progData.shutdown = true;
                  break;
                case 6:
                  Serial.print("Update Cmd from Server: ");
                  Serial.println(packet.remoteIP()[3]);
                  Serial.println(packet.data()[6]);
                  Serial.println((packet.data()[8] << 8) + packet.data()[7]);
                  progData.serverAddress=packet.remoteIP()[3];
                  progData.serverPort = (packet.data()[8] << 8) + packet.data()[7];
                  
                  config.updateConfig();
                  ESP.restart();
                  break;
                
                default:
                  break;
                }
                break;
              }
              
        
        }
      });
    }

    void udpCheck(){    // 📌  udpCheck 📝 🗑️
      if (millis() - progData.udpTimer < udpTimeout){
        progData.AOGconnected == 1;
      } else
      {
        progData.AOGconnected == 2;
      }
      
    }

    void sendHeartbeat(){
      // TODO: add heartbeat
      if (millis()-heartbeatTimePrevious > heartbeatTimeTrip){
        heartbeatTimePrevious = millis();
        
        hbData->isConnectedAOG = progData.AOGconnected;
        hbData->isConnectedCAN1 = progData.can1connected;
        hbData->isConnectedCAN2 = progData.can2connected;
        hbData->isConnectedINA219 = progData.ina219Connected;
        
        
        // int cksum=0;
        // for (int i=2;i<=sizeof(Heartbeat_t)-1;i++)
        // {
        //   cksum += heartbeat.bytes[i];
        //   // Serial.print(heartbeat.bytes[i]);
        //   // Serial.print(" ");
        // }
        udp.writeTo(heartbeat.bytes,sizeof(Heartbeat_t),IPAddress(progData.ips[0],progData.ips[1],progData.ips[2],255),9999);
        
      }
    }
 
    void sendShutdown(){
      shutdownMsg.shutdown_t.aogByte1 = 0x80;
      shutdownMsg.shutdown_t.aogByte2 = 0x81;
      shutdownMsg.shutdown_t.PGN = 149;
      shutdownMsg.shutdown_t.length = 3;
      shutdownMsg.shutdown_t.myPGN = 3;
      shutdownMsg.shutdown_t.confirm = 1;
      // udp.writeTo(shutdown.bytes,sizeof(Shutdown_t),IPAddress(Config.ips[0],Config.ips[1],Config.ips[2],255),9999);
    }
};
UDPMethods udpMethods = UDPMethods();


class CanHandler{
  private:
    
    byte TX_PIN = 1;
    byte RX_PIN = 2;

  public:
    
    byte udpBuffer[512];
    uint16_t cnt = 0;
    float dutyCycleScaleFactor = 1.6384;
    
    #pragma region CANID
      union
      {
        struct
        {
          uint8_t byte0;
          uint8_t byte1;
          uint8_t byte2;
          uint8_t byte3;
        };
        uint32_t longint;
      } canID;
    #pragma endregion
    
    CanHandler(){}


    byte startCAN(byte _RX_PIN, byte _TX_PIN){
      
      TX_PIN = _TX_PIN;
      RX_PIN = _RX_PIN;
      Serial.println("StartingCan");
      twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_NO_ACK);  // TWAI_MODE_NORMAL, TWAI_MODE_NO_ACK or TWAI_MODE_LISTEN_ONLY
      
      twai_timing_config_t t_config  = TWAI_TIMING_CONFIG_250KBITS();
      #ifndef CANFILTER
        twai_filter_config_t f_config  = TWAI_FILTER_CONFIG_ACCEPT_ALL();
      #endif
      
      #ifdef CANFILTER
        twai_filter_config_t f_config = {
          .acceptance_code = (0x18EEFF01<<3),    //Bit shift our target ID to the 29 MSBits
          .acceptance_mask = 0x7,    //Mask out our don't care bits (i.e., the 3 LSBits)
          .single_filter = true
        };
        twai_filter_config_t f_config;
        f_config.acceptance_code =   0x10EFFFD3<<3;
        f_config.acceptance_mask = ~(0x10111111<<3);
        f_config.single_filter = true;
       #endif
      
      // g_config.
      twai_driver_install(&g_config, &t_config, &f_config);
      
      if (twai_start() == ESP_OK) {
        printf("Driver started\n");
        
      } else {
        printf("Failed to start driver\n");
        return 2;
      }
      
      twai_status_info_t status;
      twai_get_status_info(&status);
      Serial.print("TWAI state ");
      Serial.println(status.state);
      return 1;
    }
    
        
    void handle_tx_message(twai_message_t message)
    {
      esp_err_t result = twai_transmit(&message, pdMS_TO_TICKS(100));
      if (result == ESP_OK){
      }
      else {
        Serial.printf("\n%s: Failed to queue the message for transmission.\n", esp_err_to_name(result));
      }
    }

    void transmit_normal_message(uint32_t identifier, uint8_t data[], uint8_t data_length_code = TWAI_FRAME_MAX_DLC)
      {
        // configure message to transmit
        twai_message_t message = {
          .flags = TWAI_MSG_FLAG_EXTD,
          .identifier = identifier,
          .data_length_code = data_length_code,
        };

        memcpy(message.data, data, data_length_code);

        //Transmit messages using self reception request
        handle_tx_message(message);
      }

    
    void checkCAN(){
      
    }

    void canRecieve(){
      twai_message_t message;
      if (twai_receive(&message, pdMS_TO_TICKS(1)) == ESP_OK) {
          canID.longint = message.identifier;
          if ((canID.byte1 == 0x00) & (canID.byte2 == 0xFF)){
            rows[0].dutyCycleCMD = (uint16_t)((message.data[3]*256+message.data[2])*dutyCycleScaleFactor);
            rows[0].frequencyCMD = (uint16_t)((message.data[1]*256+message.data[0])*.01);
            rows[0].updateValve();

            rows[1].dutyCycleCMD = (uint16_t)((message.data[7]*256+message.data[6])*dutyCycleScaleFactor);
            rows[1].frequencyCMD = (uint16_t)((message.data[5]*256+message.data[4])*.01);
            rows[1].updateValve();

          } else if ((canID.byte1 == 0x01) & (canID.byte2 == 0xFF)){
            rows[2].dutyCycleCMD = (uint16_t)((message.data[3]*256+message.data[2])*dutyCycleScaleFactor);
            rows[2].frequencyCMD = (uint16_t)((message.data[1]*256+message.data[0])*.01);
            rows[2].updateValve();

            rows[3].dutyCycleCMD = (uint16_t)((message.data[7]*256+message.data[6])*dutyCycleScaleFactor);
            rows[3].frequencyCMD = (uint16_t)((message.data[5]*256+message.data[4])*.01);
            rows[3].updateValve();

          } else if ((canID.byte1 == 0x02) & (canID.byte2 == 0xFF)){
            rows[4].dutyCycleCMD = (uint16_t)((message.data[3]*256+message.data[2])*dutyCycleScaleFactor);
            rows[4].frequencyCMD = (uint16_t)((message.data[1]*256+message.data[0])*.01);
            rows[4].updateValve();

            rows[5].dutyCycleCMD = (uint16_t)((message.data[7]*256+message.data[6])*dutyCycleScaleFactor);
            rows[5].frequencyCMD = (uint16_t)((message.data[5]*256+message.data[4])*.01);
            rows[5].updateValve();
          }
          
      }
    }
};
CanHandler canHandler = CanHandler();

#pragma region OTA
const char *errtext(int code)
{
	switch(code)
	{
		case ESP32OTAPull::UPDATE_AVAILABLE:
			return "An update is available but wasn't installed";
		case ESP32OTAPull::NO_UPDATE_PROFILE_FOUND:
			return "No profile matches";
		case ESP32OTAPull::NO_UPDATE_AVAILABLE:
			return "Profile matched, but update not applicable";
		case ESP32OTAPull::UPDATE_OK:
			return "An update was done, but no reboot";
		case ESP32OTAPull::HTTP_FAILED:
			return "HTTP GET failure";
		case ESP32OTAPull::WRITE_ERROR:
			return "Write error";
		case ESP32OTAPull::JSON_PROBLEM:
			return "Invalid JSON";
		case ESP32OTAPull::OTA_UPDATE_FAIL:
			return "Update fail (no OTA partition?)";
		default:
			if (code > 0)
				return "Unexpected HTTP response code";
			break;
	}
	return "Unknown error";
}



void OtaPullCallback(int offset, int totallength)
{
	Serial.printf("Updating %d of %d (%02d%%)...\r", offset, totallength, 100 * offset / totallength);
// #if defined(LED_BUILTIN) // flicker LED on update
// 	static int status = LOW;
// 	status = status == LOW && offset < totallength ? HIGH : LOW;
// 	digitalWrite(LED_BUILTIN, status);
// #endif
}


void softwareUpdate(){
  char basePath[] = "/%s/Releases/OTA_Config.json";
  char CONFIG_URL[150];
  sprintf(CONFIG_URL, basePath, progData.sketchConfig);
  Serial.println(CONFIG_URL);
  

  char SERVER[150];
  sprintf(SERVER, "http://%d.%d.%d.%d:%d",progData.ips[0],progData.ips[1],progData.ips[2],progData.serverAddress,progData.serverPort);

  Serial.print("CONFIG_URL: ");
  Serial.println(CONFIG_URL);
  Serial.print("SERVER: ");
  Serial.println(SERVER);
  // ota.AllowDowngrades(true);
  ota.SetConfig(progData.sketchConfig);
  ota.SetCallback(OtaPullCallback);
  
  Serial.printf("We are running version %s of the sketch, Board='%s', Device='%s', IP='%s \n", VERSION, ARDUINO_BOARD, WiFi.macAddress().c_str(),(String)(WiFi.localIP()[3]));
  Serial.println();
  // Serial.printf("Checking %s to see if an update is available...\n", CONFIG_URL);
  Serial.println();
  int ret = ota.CheckForOTAUpdate(SERVER, CONFIG_URL, VERSION);
  Serial.printf("CheckForOTAUpdate returned %d (%s)\n\n", ret, errtext(ret));
}

#pragma endregion
#pragma region Webserver

void handleFirmwareUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    Serial.printf("Update Start: %s\n", filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // Start with max available size
      Update.printError(Serial);
    }
  }
  
  // Write the received data to the flash memory
  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  }

  // If the upload is complete
  if (final) {
    if (Update.end(true)) { // True to set the size correctly
      Serial.printf("Update Success: %u bytes\n", index + len);
      request->send(200, "text/html", "Update complete! Rebooting...");
      delay(1000);
      ESP.restart();
    } else {
      Update.printError(Serial);
      request->send(500, "text/html", "Update failed.");
    }
  }
}


void updateDebugVars() {
  debugVars.clear(); // Clear the list to update it dynamically
  debugVars.push_back("Program: " + String(progData.sketchConfig));
  debugVars.push_back("Timestamp since boot [s]: " + String((float)(millis())/1000.0));
  debugVars.push_back("Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
  debugVars.push_back("Version: " + String(VERSION));
  debugVars.push_back("Wifi SSID: " + String(myWifi.ssid));
  debugVars.push_back("IP Address: " + String(progData.ips[0])+"."+String(progData.ips[1])+"."+String(progData.ips[2])+"."+String(progData.ips[3]));
  debugVars.push_back("Wifi State: " + String(progData.wifiConnected));
  debugVars.push_back("CAN 1 State: " + String(progData.can1connected));
  debugVars.push_back("Program State: " + String(progData.programState));
  debugVars.push_back("Row Data: ");
  for (int i = 0; i < NUM_OF_ROWS; i++){
    debugVars.push_back("..Section " + String(i) +":");
    debugVars.push_back("....Duty Cycle Cmd [0-100% -> 0-16384]: " + String(rows[i].dutyCycleCMD));
    debugVars.push_back("....Frequency Cmd: " + String(rows[i].frequencyCMD));
  }
}

// Function to serve the debug variables as JSON
void handleDebugVars(AsyncWebServerRequest *request) {
  updateDebugVars();  // Update the debug variables just before sending
  DynamicJsonDocument doc(1024);
  JsonArray array = doc.to<JsonArray>();
  
  for (const auto& var : debugVars) {
    array.add(var);
  }
  
  String jsonResponse;
  serializeJson(doc, jsonResponse);
  request->send(200, "application/json", jsonResponse);
}


// Function to serve the file list as JSON
void handleFileList(AsyncWebServerRequest *request) {
  DynamicJsonDocument doc(1024);
  JsonArray array = doc.to<JsonArray>();

  // Open LittleFS root directory and list files
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  
  while (file) {
    JsonObject fileObject = array.createNestedObject();
    fileObject["name"] = String(file.name());
    fileObject["size"] = file.size();
    file = root.openNextFile();
  }

  String jsonResponse;
  serializeJson(doc, jsonResponse);
  request->send(200, "application/json", jsonResponse);
}

// Reboot handler
void handleReboot(AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "Rebooting...");
  delay(100); // Give some time for the response to be sent
  ESP.restart();
}

// File download handler
void handleFileDownload(AsyncWebServerRequest *request) {
  if (request->hasParam("filename")) {
    String filename = request->getParam("filename")->value();
    if (LittleFS.exists("/" + filename)) {
      request->send(LittleFS, "/" + filename, "application/octet-stream");
    } else {
      request->send(404, "text/plain", "File not found");
    }
  } else {
    request->send(400, "text/plain", "Filename not provided");
  }
}

// File upload handler
void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    // Open file for writing
    Serial.printf("UploadStart: %s\n", filename.c_str());
    request->_tempFile = LittleFS.open("/" + filename, "w");
  }
  if (len) {
    // Write the file content
    request->_tempFile.write(data, len);
  }
  if (final) {
    // Close the file
    request->_tempFile.close();
    Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);
    request->send(200, "text/plain", "File Uploaded");
  }
}


#pragma endregion

void setSections(){
      Serial.print("Sections: ");
      for (int i=0;i<6;i++){
        rows[i].init(sectionFlowPins[i],sectionValvePins[i],(ledc_channel_t)(i));
      }
      switch(progData.ips[3]){
        case 41:
          for (int i=0;i<4;i++){
            rows[i].sectionNum = i;
          }
          break;
        case 42:
          for (int i=0;i<6;i++){
            rows[i].sectionNum = i+4;
          }
          break;
        case 43:
          for (int i=0;i<4;i++){
            rows[i].sectionNum = i+10;
          }
          break;
        case 44:
          for (int i=0;i<6;i++){
            rows[i].sectionNum = i+14;
          }
          break;
        case 45:
          for (int i=0;i<4;i++){
            rows[i].sectionNum = i+20;
          }
          break;
        
      }
      for (int i=0;i<6;i++){
        Serial.print(rows[i].sectionNum);
        Serial.print(" ");
      }
        
      
    }

void setup(){
  Serial.begin(115200);
  pinMode(POWER_RELAY_PIN, OUTPUT);
  digitalWrite(POWER_RELAY_PIN, LOW);
  config.begin();
  myLED.startTask();
  progData.programState = 1;
  int wifiWait = millis();
  while (millis()-wifiWait < 5000){
    progData.wifiConnected = myWifi.connect(progData.ips);

    if (progData.wifiConnected == 1){
      
      progData.programState = 3;
      softwareUpdate();
      progData.programState = 1;
      udpMethods.begin();
      break;
    }
  }
  if (progData.wifiConnected == 2){
    progData.wifiConnected = myWifi.makeAP(progData.ips,progData.sketchConfig);
    
  }
  if ((progData.wifiConnected == 1) || (progData.wifiConnected == 3)){
  #pragma region Server Setup
        // Serve the main HTML page
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
          request->send(LittleFS, "/index.html");
        });
        // Route to get debug variables as JSON
        server.on("/getDebugVars", HTTP_GET, handleDebugVars);
        // Route to list files as JSON
        server.on("/listFiles", HTTP_GET, handleFileList);
        // Route to download files
        server.on("/download", HTTP_GET, handleFileDownload);

        // Handle file upload
        server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {}, handleFileUpload);

        server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {}, 
  handleFirmwareUpload);

        server.on("/reboot", HTTP_GET, handleReboot);
        
        // Start server
        server.begin();
      #pragma endregion
}
  progData.can1connected = canHandler.startCAN(CAN1_RX_PIN, CAN1_TX_PIN);
  setSections();
  digitalWrite(POWER_RELAY_PIN, HIGH);
  progData.programState = 1;
  
  Serial.println("Setup Complete");
}


void loop(){
  if (progData.can1connected == 1){
    canHandler.canRecieve();
  }
 
}