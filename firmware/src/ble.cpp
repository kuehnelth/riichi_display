#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include "ble.h"

#define DISPLAY_SERVICE_UUID "bae5e4dd-f2b4-4461-a84c-b7851fb8efd3"
#define GAME_STATE_CHARACTERISTIC_UUID "bab40271-33ea-48dc-a145-638361f54d2b"

static BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

class ServerCallbacks : public BLEServerCallbacks {
	void onConnect(BLEServer *pServer)
	{
		deviceConnected = true;
	};
	void onDisconnect(BLEServer *pServer)
	{
		deviceConnected = false;
		// restart advertising after disconnecting
		pServer->startAdvertising();
	}
};

class Callbacks : public BLECharacteristicCallbacks {
	void onWrite(BLECharacteristic *pCharacteristic)
	{
		String value = pCharacteristic->getValue();
		char *str = strdup(value.c_str());
		char *bak = str;
		free(bak);
	}
};

void init_ble(void)
{
	BLEDevice::init("Riichi Display");
	BLEServer *pServer = BLEDevice::createServer();
	pServer->setCallbacks(new ServerCallbacks());
	BLEService *pService = pServer->createService(DISPLAY_SERVICE_UUID);

	pCharacteristic = pService->createCharacteristic(
		GAME_STATE_CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_WRITE);

	pCharacteristic->setCallbacks(new Callbacks());
	pService->start();

	BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
	pAdvertising->addServiceUUID(DISPLAY_SERVICE_UUID);
	pAdvertising->start();
	//Serial.println("BLE server started, advertising...");
}

void deinit_ble(void)
{
	BLEDevice::deinit(true);
}
