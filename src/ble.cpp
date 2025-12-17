#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string.h>
#include "game_state.h"
#include "display.h"

#define DISPLAY_SERVICE_UUID "bae5e4dd-f2b4-4461-a84c-b7851fb8efd3"
#define GAME_STATE_CHARACTERISTIC_UUID "bab40271-33ea-48dc-a145-638361f54d2b"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

extern char names[4][32];
extern int scores[4];
extern int state[3];

class MyServerCallbacks : public BLEServerCallbacks {
	void onConnect(BLEServer *pServer)
	{
		deviceConnected = true;
		//Serial.println("connect");
	};
	void onDisconnect(BLEServer *pServer)
	{
		deviceConnected = false;
		pServer->startAdvertising(); // restart advertising after disconnecting
		//Serial.println("disconnect");
		connectScreen();
	}
};

class MyCallbacks : public BLECharacteristicCallbacks {
	void onWrite(BLECharacteristic *pCharacteristic)
	{
		//Serial.println("Client is writing the value");
		String value = pCharacteristic->getValue();
		//Serial.print(value);
		char *str = strdup(value.c_str());
		char *bak = str;
		int type = 0;
		int item = 0;
		uint8_t wind = 0;

		int res, id, score;
		while (strlen(str)) {
			char *next = strchr(str, '\n');
			if (!next)
				break;
			next[0] = '\0';

			if (item == 0) {
				sscanf(str, "%" SCNu8 " %" SCNu8 " %" SCNu8, &game_state.round,
				       &game_state.riichi_count, &game_state.honba_count);
				state[type] = atoi(str);
				//Serial.printf("state[%d] = %d\n", type, state[type]);
				item++;
				type = 0;
			} else {
				Player *player;
				switch (type) {
				case 0:
					id = atoi(str);
					wind = id;
					break;
				case 1:
					score = atoi(str);
					break;
				case 2:
					player = &game_state.players[item - 1];
					strcpy(player->name, str);
					player->score = score;
					player->wind = (Wind)wind;
					strcpy(names[id], str);
					scores[id] = score;
					//Serial.printf("id %d name %s s %d\n", id, str, score);
					break;
				default:
					break;
				}
				type = (++type) % 3;
				if (type == 0)
					item++;
			}
			str = &next[1];
		}
		free(bak);
		showFullUpdate();
	}
	void onRead(BLECharacteristic *pCharacteristic)
	{
		//Serial.println("Client is reading the value");
	}
};

void setupBle()
{
	BLEDevice::init("Riichi Display");
	BLEServer *pServer = BLEDevice::createServer();
	pServer->setCallbacks(new MyServerCallbacks());
	BLEService *pService = pServer->createService(DISPLAY_SERVICE_UUID);

	pCharacteristic = pService->createCharacteristic(
		GAME_STATE_CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_WRITE);

	pCharacteristic->setCallbacks(new MyCallbacks());
	pService->start();

	BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
	pAdvertising->addServiceUUID(DISPLAY_SERVICE_UUID);
	pAdvertising->start();
	//Serial.println("BLE server started, advertising...");
}

void stopBle(void)
{
	BLEDevice::deinit(true);
}
