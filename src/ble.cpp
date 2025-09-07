#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <cstdlib>
#include <cstring>
#include <string.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
int value = 0;

extern char names[4][32];
extern int scores[4];

void showPartialUpdate(void);

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("connect");
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      pServer->startAdvertising(); // restart advertising after disconnecting
      Serial.println("disconnect");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        Serial.println("Client is writing the value");
        String value = pCharacteristic->getValue();
        //Serial.print(value);
        char *str = strdup(value.c_str());
        char *bak = str;
        int type = 0;

        int res,id, score;
        while (strlen(str)) {
          char *next = strchr(str, '\n');
          if (!next)
            break;
          next[0] = '\0';

          switch (type) {
            case 0:
              id = atoi(str);
              break;
            case 1:
              score = atoi(str);
              break;
            case 2:
              strcpy(names[id], str);
              scores[id] = score;
              Serial.printf("id %d name %s s %d\n", id, str, score);
              break;
            default:
              break;
          }
          str = &next[1];
          type = (++type) % 3;

        }
        free(bak);
        showPartialUpdate();
    }
    void onRead(BLECharacteristic* pCharacteristic) {
        Serial.println("Client is reading the value");
    }


};

void setupBle() {
  BLEDevice::init("ESP32 BLE Server");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
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
