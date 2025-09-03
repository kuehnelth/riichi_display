#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>
#include <StreamString.h>

#include <NetworkClientSecure.h>

#include "proto/mimir.pb.h"
#include <pb.h>
#include <pb_common.h>
#include <pb_decode.h>

#include "display.h"
#include "ble.h"

WiFiMulti WiFiMulti;

void setup()
{
	Serial.begin(115200);
	// Serial.setDebugOutput(true);

	setupBle();
	Serial.print("init display");
	init_display();

	/*
	WiFi.mode(WIFI_STA);
	WiFiMulti.addAP("\\oO/", "supersicher0815!!!");

	// wait for WiFi connection
	Serial.print("Waiting for WiFi to connect...");
	while ((WiFiMulti.run() != WL_CONNECTED)) {
		Serial.print(".");
	}
	Serial.println(" connected");
	*/
}

static GamesGetSessionOverviewResponse resp = GamesGetSessionOverviewResponse_init_zero;
void loop()
{
	delay(1000000);
	return;
	NetworkClientSecure *client = new NetworkClientSecure;
	if (client) {
		// client->setCACert(rootCACertificate);
		client->setInsecure();

		{
			// Add a scoping block for HTTPClient https to make sure it is destroyed
			// before NetworkClientSecure *client is
			HTTPClient https;

			Serial.print("[HTTPS] begin...\n");
			if (https.begin(*client,
					"https://gameapi.riichimahjong.org/v2/"
					"common.Mimir/GetSessionOverview")) { // HTTPS
				// if
				// (https.begin("http://9001.ovh:8000/v2/common.Mimir/GetCurrentStateForPlayer"))
				// {  // HTTPS
				Serial.print("[HTTPS] GET...\n");
				// start connection and send HTTP header
				https.addHeader("Content-Type", "application/protobuf");
				https.addHeader("x-current-person-id", "828");
				https.addHeader("x-twirp", "true");
				https.addHeader("x-auth-token", "2066921bd8b7dec022fe0d783aefdc32e891fb3593eb9580470736"
								"2af10209f9b85abeeb7005e5b617ae16bc9a4aee6f");

				uint8_t data[] = { 0x08, 0x8a, 0x05, 0x10, 0xbc, 0x06 };
				// int httpCode = https.POST(data, 6);
				int httpCode = https.POST("\n(a6013d92baafd7d6286bac9e6bcb316915ccf35d");

				// httpCode will be negative on error
				if (httpCode > 0) {
					// HTTP header has been send and Server response header has been
					// handled
					Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

					// file found at server
					if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
						// String payload = https.getString();
						// Serial.println(payload);
						StreamString s;
						uint8_t buf[1024];

						int len = https.writeToStream(&s);
						s.readBytes(buf, len);
						Serial.printf("len: %d\n", len);

						/* Create a stream that reads from the buffer. */
						pb_istream_t stream = pb_istream_from_buffer(buf, len);

						/* Now we are ready to decode the message. */
						int status = pb_decode(&stream, GamesGetSessionOverviewResponse_fields,
								       &resp);
						Serial.printf("status: %d\n", status);
						if (status) {
							// Serial.println(resp.sessions_count);
							// for (int i=0; i < resp.sessions_count; i++) {
							Serial.printf(
								"Session dealer %d round_index %d riichi_count %d "
								"honba_count %d\n",
								resp.state.dealer, resp.state.round_index,
								resp.state.riichi_count, resp.state.honba_count);
							for (int j = 0; j < resp.players_count; j++)
								Serial.printf("id %d name %s score %d\n",
									      resp.players[j].id, resp.players[j].title,
									      resp.players[j].score);
							//}
						}
					}
				} else {
					Serial.printf("[HTTPS] GET... failed, error: %s\n",
						      https.errorToString(httpCode).c_str());
					String payload = https.getString();
					Serial.println(payload);
				}

				https.end();
			} else {
				Serial.printf("[HTTPS] Unable to connect\n");
			}

			// End extra scoping block
		}

		delete client;
	} else {
		Serial.println("Unable to create client");
	}

	Serial.println();
	Serial.println("Waiting 10s before the next round...");
	delay(1000000);
}
