// #define CANFILTER

#include <Arduino.h>
#include <myWifi.h>
// #include <myPreferences.h>
#include <driver/twai.h>
#include <driver/gpio.h>
#include "AsyncUDP.h"
#include "Wire.h"
#include "LittleFS.h"
#include <ArduinoJSON.h>
#include "ESP32OTAPull.h"
#include "Preferences.h"
#include "Version.h"
#include <WebServer.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <messageStructs.h>
#include <Adafruit_NeoPixel.h>
#include <vector>
#include <SPIFFS.h>
// #include <ESPAsyncWebSocket.h>
// put function declarations here:

#define SDA_0 5
#define SCL_0 4
#ifdef CAN1
  #define CAN1_RX_PIN 2
  #define CAN1_TX_PIN 1
#endif
#ifdef CAN2
  #define CAN1_RX_PIN 41
  #define CAN1_TX_PIN 42
#endif


#define POWER_RELAY_PIN 14
#define debugPrintInterval 250
#define HEARBEAT_PGN 5
#define MY_PGN 149
#define LH_OUTER_FLIP_CAP 11
#define LH_OUTER_FLIP_ROD 1
#define LH_WING_ROTATE_OUT 2
#define LH_WING_ROTATE_IN 3
#define LH_WING_RAISE 4
#define RH_WING_RAISE 5
#define CENTER_RAISE 6

#define RH_WING_ROTATE_IN 7
#define RH_WING_ROTATE_OUT 8
#define RH_OUTER_FLIP_CAP 9
#define RH_OUTER_FLIP_ROD 10
#define DIRECTIONAL_VALVE 0


#define lhOuterWingRotate 0
#define lhWingRotate 1
#define lhWingLift 2
#define centerLift 3
#define rhWingLift 4
#define rhWingRotate 5
#define rhOuterWingRotate 6

String foldNames[] = {"lhOuter","lhRotate","lhLift","center","rhLift","rhRotate","rhOuter"};
uint8_t udpFoldCmds[7];
uint8_t foldCmds[7];
unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;
// uint8_t foldPins[] = {13,12,11,10,9,8,18,17,16,15,7,6};
uint8_t foldPins1[] = {12,11,15,16,17,18,9};
uint8_t foldPins2[] = {6,7,15,16,17,10,8};
uint8_t directionalValvePin = 13;

#pragma region GlobalClasses
  // MyPrefrences myPrefs;
  TaskHandle_t LEDtask;
  TaskHandle_t CheckertTask;
  Adafruit_NeoPixel pixels(1, 48, NEO_GRB + NEO_KHZ800);
  MyWifi myWifi;
  String header;
  ESP32OTAPull ota;
  AsyncWebServer server(80);
  std::vector<String> debugVars;
  // AsyncWebSocket ws("/ws");
#pragma endregion

#pragma region GlobalStructs
  struct ProgramVars_t{
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
  } progVars;

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
  } heartbeat;
  Heartbeat_t * hbData = &heartbeat.heartbeat_t;
#pragma endregion

// void softwareUpdate();

class Configuration{
  private:
    const size_t capacity = JSON_OBJECT_SIZE(10) + 200;
    
  public:
    char sketchConfig[64];
    byte ips[4];
    uint16_t serverPort;
    byte serverAddress;
    // Preferences prefs;
    Configuration(){}

    bool begin() {
      if (!LittleFS.begin(true)) {
        Serial.println("An error has occurred while mounting LittleFS");
        return false;
      }
      Serial.println("LittleFS mounted successfully");

      File file = LittleFS.open("/config.json", "r");
      if (!file) {
        Serial.println("Failed to open file for reading");
        return false;
      }

      String jsonString;
      while (file.available()) {
          jsonString += char(file.read());
      }
      file.close();
      
      // Print the JSON string to verify
      Serial.println(jsonString);

      
      DynamicJsonDocument doc(capacity);

      // Parse the JSON string
      DeserializationError error = deserializeJson(doc, jsonString);
      if (error) {
          Serial.print("Failed to parse JSON: ");
          Serial.println(error.c_str());
          return false;
      }
      // Access the JSON data
      
      strlcpy(sketchConfig, doc["Config"],sizeof(sketchConfig)); 
      Serial.print("Sketch Config: ");
      Serial.println(sketchConfig);
      ips[0] = int(doc["ip1"]);
      ips[1] = int(doc["ip2"]);
      ips[2] = int(doc["ip3"]);
      ips[3] = int(doc["ip4"]);
      serverAddress = int(doc["serverAdr"]);
      serverPort = int(doc["serverPort"]);
      return true;
    }

    bool updateConfig(){
      File file = LittleFS.open("/config.json", "w");
      if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
      }
      DynamicJsonDocument doc(capacity);
      doc["Config"] = sketchConfig;
      doc["ip1"] = ips[0];
      doc["ip2"] = ips[1];
      doc["ip3"] = ips[2];
      doc["ip4"] = ips[3];
      doc["serverAdr"] = serverAddress;
      doc["serverPort"] = serverPort;
      // Parse the JSON string
      
      
      if (serializeJson(doc, file) == 0) {
        Serial.println(F("Failed to write to file"));
      }

      // Close the file
      file.close();
      return true;
    }
};
Configuration Config = Configuration();

class DirectionalValve{
  public:
    DirectionalValve(){}
    uint8_t pin1;
    uint8_t valveState;
    void init(){
      pinMode(pin1, OUTPUT);
      digitalWrite(pin1, LOW);
    }
    void loop(){
      if (valveState > 0){
        digitalWrite(pin1, HIGH);
      } else {
        digitalWrite(pin1, LOW);
      }
    }
};
DirectionalValve dirValve = DirectionalValve();

class FoldValve{
  public:
    uint8_t pin1;
    uint8_t pin2;
    uint8_t valveState;
    FoldValve(){}
    void init(){
      pinMode(pin1, OUTPUT);
      pinMode(pin2, OUTPUT);
      digitalWrite(pin1, LOW);
      digitalWrite(pin2, LOW);
    }
    void loop(){
      if (valveState == 0){
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, LOW);
      } else if (valveState == 1){
        digitalWrite(pin1, HIGH);
        digitalWrite(pin2, HIGH);
      }
    }
};
FoldValve foldValves[7];

class FoldController{
  private:
    
  public:
    FoldController(){}

    void init(){
    }

    void loop(){
      logicController();
    }
    
    void logicController(){
      int stateCnt = 0;
      for (int i=0; i<7;i++){
        foldCmds[i]=udpFoldCmds[i];
        if (foldCmds[i]==1){
          foldValves[i].valveState = 1;
        } else if (foldCmds[i] == 0 ) {
          foldValves[i].valveState = 0;
        } else if (foldCmds[i] == 2) {
          foldValves[i].valveState = 1;
          stateCnt++;
        }
      }
      if (stateCnt > 0){
        dirValve.valveState = 1;
      } else if (stateCnt == 0){
        dirValve.valveState = 0;
      }
    }
};
FoldController foldCtrl = FoldController();

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
          progVars.udpTimer = millis();
          switch (packet.data()[3]){
            case 201:
              
              Config.ips[0] = packet.data()[7];
              Config.ips[1] = packet.data()[8];
              Config.ips[2] = packet.data()[9];
              Config.updateConfig();
              ESP.restart();
              break;
          
            case 149:
              switch (packet.data()[5])
                {
                case 5:
                  progVars.shutdown = true;
                  break;
                case 6:
                  Serial.print("Update Cmd from Server: ");
                  Serial.println(packet.remoteIP()[3]);
                  Serial.println(packet.data()[6]);
                  Serial.println((packet.data()[8] << 8) + packet.data()[7]);
                  Config.serverAddress=packet.remoteIP()[3];
                  Config.serverPort = (packet.data()[8] << 8) + packet.data()[7];
                  
                  Config.updateConfig();
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
      if (millis() - progVars.udpTimer < udpTimeout){
        progVars.AOGconnected == 1;
      } else
      {
        progVars.AOGconnected == 2;
      }
      
    }

    void sendHeartbeat(){
      // TODO: add heartbeat
      if (millis()-heartbeatTimePrevious > heartbeatTimeTrip){
        heartbeatTimePrevious = millis();
        
        
        
        
        // int cksum=0;
        // for (int i=2;i<=sizeof(Heartbeat_t)-1;i++)
        // {
        //   cksum += heartbeat.bytes[i];
        //   // Serial.print(heartbeat.bytes[i]);
        //   // Serial.print(" ");
        // }
        udp.writeTo(heartbeat.bytes,sizeof(Heartbeat_t),IPAddress(Config.ips[0],Config.ips[1],Config.ips[2],255),9999);
        
      }
    }
 
    
};
UDPMethods udpMethods = UDPMethods();
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
  
  
  // String _CONFIG_URL = String(config.ips[0]) + "." + String(config.ips[1]) + "." + String(config.ips[2]) + "." + String(config.serverAddress) + ":" + String(config.serverPort) + "/" + String(config.sketchConfig) + "/" + "Releases" + "/" + "OTA_Config.json";
  // char* CONFIG_URL;
  // _CONFIG_URL.toCharArray(CONFIG_URL, sizeof(_CONFIG_URL)+1);
  // Serial.println(_CONFIG_URL);
  
  char CONFIG_URL[150];
  sprintf(CONFIG_URL, "http://%d.%d.%d.%d:%d/ISO_Monitor/Releases/OTA_Config.json",Config.ips[0],Config.ips[1],Config.ips[2],Config.serverAddress,Config.serverPort);
  Serial.println(CONFIG_URL);
  String _SERVER = String(Config.ips[0]) + "." + String(Config.ips[1]) + "." + String(Config.ips[2]) + "." + String(Config.serverAddress) + ":" + String(Config.serverPort);
  char SERVER[150];
  _SERVER.toCharArray(SERVER, sizeof(_SERVER) + 1);
  // 

  // #define CONFIG_URL_TEMP "http://192.168.8.249:5501/ISO_Monitor/Releases/OTA_Config.json"
  
  ota.SetConfig(Config.sketchConfig);
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
void updateDebugVars() {
  debugVars.clear(); // Clear the list to update it dynamically
  debugVars.push_back("Timestamp since boot [ms]: " + String(millis()));
  debugVars.push_back("Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
  debugVars.push_back("Program: " + String(Config.sketchConfig));
  debugVars.push_back("Version: " + String(VERSION));
  debugVars.push_back("Wifi SSID: " + String(myWifi.ssid));
  debugVars.push_back("Wifi IP: " + String(WiFi.localIP().toString()));
  debugVars.push_back("Wifi State: " + String(progVars.wifiConnected));
  debugVars.push_back("Program State: " + String(progVars.programState));
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

class DebugPrinter{
  private:

  public:
    int lastPrint = 0;
    String debugString;
    DebugPrinter(){
    
    }
         
    
    
    void loop(){
      if (millis()-lastPrint > debugPrintInterval){
        lastPrint = millis();
        debugString = "";
        debugString = debugString + String(millis()) + " ";
        debugString = debugString +"Version: ";
        debugString = debugString + String(VERSION);
        debugString = debugString+ " IP State: ";
        debugString = debugString + String(progVars.wifiConnected);
        if (progVars.wifiConnected == 1){
          debugString = debugString+ " IP Add: ";
          debugString = debugString + String(Config.ips[0])+"."+String(Config.ips[1])+"."+String(Config.ips[2])+"."+String(Config.ips[3]);
          debugString = debugString+ " Server Info: ";
          debugString = debugString+ "http:\\"+String(Config.ips[0])+"."+String(Config.ips[1])+"."+String(Config.ips[2])+"."+String(Config.serverAddress)+":"+String(Config.serverPort);
        }
        debugString = debugString+ " CAN 1: ";
        debugString = debugString + String(progVars.can1connected);
        debugString = debugString + " INA219: ";
        debugString = debugString + String(progVars.ina219Connected);
        Serial.print(debugString);
        Serial.println();
        
        
        
        
        
        if(!WiFi.isConnected()){
          progVars.wifiConnected == 2;
          progVars.wifiConnected = myWifi.connect(Config.ips);

        }

      }
      
      
    }
};
DebugPrinter debugPrint = DebugPrinter();



void LEDloopCode( void * parameter) {
  long firstPixelHue = 0;
  uint32_t updateTimer = 0;
  // uint32_t cnt = 0;
  for(;;) {
    switch (progVars.programState){
      case 1:
        // int pixelHue = firstPixelHue;
        pixels.setPixelColor(0, pixels.gamma32(pixels.ColorHSV(firstPixelHue)));
        pixels.show();
        firstPixelHue = firstPixelHue + 256;
        if (firstPixelHue > 5*65536){
          firstPixelHue = 0;
        }
        // // rainbow(20);
        break;
      case 2:
        pixels.setPixelColor(0,pixels.Color(0,255,0));
        pixels.show();
        break;
      case 3:
        if (millis()-updateTimer < 250){
          pixels.setPixelColor(0,0x00ffffff);
          pixels.show();
        } else if ((millis()-updateTimer > 250) && (millis()-updateTimer < 500))
        {
          pixels.setPixelColor(0,0x000000ff);
          pixels.show();
        } else {
          updateTimer = millis();
        }
        

        break;
      default:
        break;
    }
   
    delay(5);
  }
}

void CheckerCode( void * parameter) {
  uint32_t checkTimer = 0;
  // uint32_t cnt = 0;
  for(;;) {
    if (millis()-checkTimer > 1000){
      checkTimer = millis();
      if (!WiFi.isConnected()){
        myWifi.connect(Config.ips);
      }
    }
    vTaskDelay(100);
    
  }
}

void setup(){
  Serial.begin(115200);
  progVars.programState = 1;
  pixels.begin();
  pixels.setBrightness(50);
  // delay(1000);
  xTaskCreatePinnedToCore(
      LEDloopCode, /* Function to implement the task */
      "LEDloop", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &LEDtask,  /* Task handle. */
      0); /* Core where the task should run */
  
  Wire.setPins(5,4);
  Wire.begin();
  Config.begin();
    Serial.println("ESP32 Module:");
    Serial.print("\tSketch Name: ");
    Serial.println(Config.sketchConfig);
    Serial.print("\tVersion: ");
    Serial.println(VERSION);
  
  while (true){
    progVars.wifiConnected = myWifi.connect(Config.ips);
    if (progVars.wifiConnected == 1){
      break;
    }
  }
  
  // server.on("/", handleRoot); 
  if (progVars.wifiConnected == 1){
    
    Serial.println(String(Config.ips[0])+"."+String(Config.ips[1])+"."+String(Config.ips[2])+"."+String(Config.ips[3]));
    progVars.programState = 3;
    softwareUpdate();
    progVars.programState = 1;
    udpMethods.begin();
    

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

    server.on("/reboot", HTTP_GET, handleReboot);
    // Start server
    server.begin();
    for (int i=0; i<7;i++){
      foldValves[i]=FoldValve();
      foldValves[i].pin1 = foldPins1[i];
      foldValves[i].pin2 = foldPins2[i];
      foldValves[i].init();
    }
    dirValve.pin1 = directionalValvePin;
    dirValve.init();
    digitalWrite(POWER_RELAY_PIN, HIGH);
    foldCtrl.init();
  }
  
  
  
  progVars.programState = 2;
  xTaskCreatePinnedToCore(
      CheckerCode, /* Function to implement the task */
      "Checkerloop", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &CheckertTask,  /* Task handle. */
      0); /* Core where the task should run */
  Serial.println("Setup Complete");

}

void loop(){
  
  if (progVars.wifiConnected == 1){
    udpMethods.udpCheck();
    udpMethods.sendHeartbeat();
  }
  
  debugPrint.loop();
  

}