#ifndef BLUETOOTHCOMMS_H
#define BLUETOOTHCOMMS_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

class BluetoothRemote {
public:
    BluetoothRemote();  // Constructor
    void initBLE(const char* deviceName);  // Initialize BLE
    void sendIPData(const std::string& ipValue);  // Send sensor data as a BLE notification
    // String getIP();

private:
    BLEServer* pServer;
    BLECharacteristic* ipCharacteristic;
    BLECharacteristic* newIPCharacteristic;
    BLEAdvertising* pAdvertising;
    // Define the server callbacks class for connection handling
    class ServerCallbacks : public BLEServerCallbacks {
        void onConnect(BLEServer* pServer) override;
        void onDisconnect(BLEServer* pServer) override;
    };
    // String newIP;
    static void newIPCallback(BLECharacteristic* pCharacteristic);
};


#endif