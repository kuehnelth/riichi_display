#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <cstring>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
int value = 0;

extern char names[4][16];
extern int scores[4];

void showPartialUpdate(void);

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        String value = pCharacteristic->getValue();
        int res,id, score;
        char name[16];
        res = sscanf(value.c_str(), "%d %d %s", &id, &score, name);
        if (res == 3) {
            strcpy(names[id], name);
            scores[id] = score;
            Serial.printf("Client is writing the value %s\n", value);
        }
    }
    void onRead(BLECharacteristic* pCharacteristic) {
        Serial.println("Client is reading the value");
        showPartialUpdate();
    }
};

void setupBle() {
  BLEDevice::init("ESP32 BLE Server");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("Hello BLE Client");
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  Serial.println("BLE server started, advertising...");
}
