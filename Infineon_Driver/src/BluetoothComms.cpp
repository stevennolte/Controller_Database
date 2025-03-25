

#include "BluetoothComms.h"



BluetoothRemote::BluetoothRemote() {
    pServer = nullptr;
    ipCharacteristic = nullptr;
    pAdvertising = nullptr;
}

void BluetoothRemote::initBLE(const char* deviceName) {
    BLEDevice::init(deviceName);
    pServer = BLEDevice::createServer();

    BLEService* remoteService = pServer->createService("0d906332-7b60-4392-a6f2-097afb163897");

    
    // Sensor characteristic (for reading sensor data)
    ipCharacteristic = remoteService->createCharacteristic(
        "a6f580b4-c5f5-43fd-b186-06547820061b",
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );

    remoteService->start();
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(remoteService->getUUID());
    pAdvertising->start();
}



void BluetoothRemote::sendIPData(const std::string& ipValue) {
    if (ipCharacteristic) {
        ipCharacteristic->setValue(ipValue);
        ipCharacteristic->notify();  // Send as BLE notification
    }
}
