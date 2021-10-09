#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <wifi_storage.hpp>

class wifiStoreCallback : public BLECharacteristicCallbacks
{
public:
    wifiStoreCallback(WifiCredentialStore *wifiStore) : store{wifiStore} {}

protected:
    WifiCredentialStore* store;
};

class SSIDCallback : public wifiStoreCallback
{
public:
    SSIDCallback(WifiCredentialStore *store) : wifiStoreCallback(store) {}
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        //write characteristic to storage
        Serial.println("writing new ssid to storage");
        auto newSSID = pCharacteristic->getValue();
        store->storeSSID(newSSID.c_str());
    }

};

class PassCallback : public wifiStoreCallback
{
public:
    PassCallback(WifiCredentialStore *store) : wifiStoreCallback(store) {}
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        Serial.println("writing new pass to storage");
        store->storePass(pCharacteristic->getValue().c_str());
    }
};

class MbtaStopCallback : public wifiStoreCallback
{
public:
    MbtaStopCallback(WifiCredentialStore *store) : wifiStoreCallback(store) {}
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        Serial.println("writing new pass to storage");
        store->storeStop(atoi(pCharacteristic->getValue().c_str()));
    }
};

class BleSystem
{
public:
    BleSystem()
    {
        Serial.println("creating ble device");
        BLEDevice::init("ESP32");
        Serial.println("creating service");
        BLEServer *pServer = BLEDevice::createServer();
        pService = pServer->createService("4fafcx01-1fb5-459e-8fcc-c5c9c331914b");
        Serial.println("creating characteristic");
        store = WifiCredentialStore();
        auto creds = store.getCredentials();
        makeSSIDCharacteristic(creds.ssid);
        makePassCharacteristic(creds.pass);
        makeStopCharacteristic(creds.mbtaStop);
    }

    void startServer()
    {
        Serial.println("starting ble server");
        pService->start();
        //BLEAdvertising *pAdvertising = pServer->getAdvertising();
        BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pAdvertising->setScanResponse(true);
        pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
        pAdvertising->setMinPreferred(0x12);
        Serial.println("starting advertisement");
        BLEDevice::startAdvertising();
        //pAdvertising->start();
        Serial.println("Characteristic defined! Now you can read it in the Client!");
    }

private:
    BLEServer *pServer;
    BLEService *pService;
    BLECharacteristic *pPassCharacteristic;
    BLECharacteristic *pSSIDCharacteristic;
    const char *SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
    const char *CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
    WifiCredentialStore store;


    void makeSSIDCharacteristic(char* initialSSID)
    {
        pPassCharacteristic = pService->createCharacteristic(
            "beb5483e-36e1-4688-b7f5-ea07361sl6a8",
            BLECharacteristic::PROPERTY_READ |
                BLECharacteristic::PROPERTY_WRITE);
        Serial.println("created service");
        pPassCharacteristic->setValue(initialSSID);
        pPassCharacteristic->setCallbacks(new SSIDCallback(&store));
        
    }

    void makePassCharacteristic(char* initialPass)
    {
        pPassCharacteristic = pService->createCharacteristic(
            "beb5483e-36e1-4688-b7f5-ea07361b26a8",
            BLECharacteristic::PROPERTY_READ |
                BLECharacteristic::PROPERTY_WRITE);
        Serial.println("created service");
        pPassCharacteristic->setValue(initialPass);
        pPassCharacteristic->setCallbacks(new PassCallback(&store));

    }

    void makeStopCharacteristic(int initialStop)
    {
        pPassCharacteristic = pService->createCharacteristic(
            "bebls03e-36e1-4688-b7f5-ea07361b26a8",
            BLECharacteristic::PROPERTY_READ |
                BLECharacteristic::PROPERTY_WRITE);
        Serial.println("created service");
        String stop;
        stop.concat(initialStop);
        pPassCharacteristic->setValue(stop.c_str());
        pPassCharacteristic->setCallbacks(new MbtaStopCallback(&store));

    }

};