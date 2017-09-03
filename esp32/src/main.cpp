#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

int output_pin = 5;
#define PIN            15
#define NUMPIXELS      29
const char* ssid = "morty";
const char* password = "***REMOVED***";
const char* mqtt_server = "mqtt.chris.gunawardena.id.au";
uint32_t rgbHex = 0;
int brightness = 0;
// const char* mqtt_user = "mqtt_user";
// const char* mqtt_password = "mqtt_secret";

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGBW + NEO_KHZ800);
bool on = false;

const char* sync_res_template = "{\"requestId\":\"xxxx\",\"payload\":{\"devices\":[{\"id\":\"456\",\"type\":\"action.devices.types.LIGHT\",\"traits\":[\"action.devices.traits.OnOff\",\"action.devices.traits.Brightness\",\"action.devices.traits.ColorSpectrum\"],\"name\":{\"name\":\"kitchen light\"},\"willReportState\":true}]}}";
const char* query_res_template = "{\"requestId\":\"zzzz\",\"payload\":{\"devices\":{\"456\":{\"online\":true,\"on\":false,\"brightness\":65,\"color\":{\"name\":\"lamp1\",\"spectrumRGB\":16510692}}}}}";
const char* exec_res_template = "{\"requestId\":\"zzzz\",\"payload\":{\"commands\":[{\"ids\":[\"456\"],\"status\":\"SUCCESS\",\"states\":{\"on\":true,\"online\":true}}]}}";

char* string2char(String command){
		if(command.length()!=0){
				char *p = const_cast<char*>(command.c_str());
				return p;
		}
}

void set_strip_color(int r, int g, int b, int w) {
	for (int i=0; i<NUMPIXELS; i++) {  
		pixels.setPixelColor(i, pixels.Color(r,g,b,w));
	}
	pixels.show();
	delay(1);
}

void setup_wifi() {
	delay(10);
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	randomSeed(micros());
	Serial.println("WiFi connected");
}

void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
	}
	Serial.println();

	if (strcmp(topic, "/lights/kitchen") == 0) {
		DynamicJsonBuffer req_buffer(1024);    
		JsonObject& req_root = req_buffer.parseObject((char *) payload, 20);
		const char* reply_id = req_root["reply_id"]; // "/reply/id_8106"
		const char* requestId = req_root["body"]["requestId"];
		const char* intent = req_root["body"]["inputs"][0]["intent"]; // "action.devices.SYNC"		
		DynamicJsonBuffer res_buffer(1024);      
		if (strcmp(intent, "action.devices.EXECUTE") == 0) {
			const char* command = req_root["body"]["inputs"][0]["payload"]["commands"][0]["execution"][0]["command"]; // "action.devices.commands.OnOff"
 			if (strcmp(command, "action.devices.commands.OnOff") == 0) {
				on = req_root["body"]["inputs"][0]["payload"]["commands"][0]["execution"][0]["params"]["on"];
				if(on) {
					digitalWrite(output_pin, HIGH);
					set_strip_color(0, 0, 0, 255);
				} else {
					digitalWrite(output_pin, LOW);
					set_strip_color(0, 0, 0, 0);
				}
			} else if (strcmp(command, "action.devices.commands.ColorAbsolute") == 0) {
				rgbHex = req_root["body"]["inputs"][0]["payload"]["commands"][0]["execution"][0]["params"]["color"]["spectrumRGB"];
				set_strip_color((rgbHex >> 8) & 0xFF, (rgbHex >> 16) & 0xFF, rgbHex & 0xFF, 0);
			}  else if (strcmp(command, "action.devices.commands.BrightnessAbsolute") == 0) {
				brightness = req_root["body"]["inputs"][0]["payload"]["commands"][0]["execution"][0]["params"]["brightness"];
				brightness = (brightness * 255) / 100;
				set_strip_color(0, 0, 0, brightness);
			} else {
				Serial.println("NO cmd MATCH");
			}
			JsonObject& res_root = res_buffer.parseObject(exec_res_template);
			res_root["requestId"] = requestId;
			String res_str;
			res_root.printTo(res_str);
			Serial.println(res_str.c_str());
			client.publish(reply_id, res_str.c_str());      
		} else if (strcmp(intent, "action.devices.SYNC") == 0) {
			JsonObject& res_root = res_buffer.parseObject(sync_res_template);
			res_root["requestId"] = requestId;
			res_root["payload"]["devices"]["456"]["on"] = on; // false
			res_root["payload"]["devices"]["456"]["brightness"] = (brightness * 100) / 255;
			res_root["payload"]["devices"]["456"]["color"]["spectrumRGB"] = rgbHex;
			String res_str;
			res_root.printTo(res_str);
			Serial.println(res_str.c_str());
			client.publish(reply_id, res_str.c_str());      
		} else if (strcmp(intent, "action.devices.QUERY") == 0) {
			JsonObject& res_root = res_buffer.parseObject(query_res_template);
			res_root["requestId"] = requestId;
			String res_str;
			res_root.printTo(res_str);
			Serial.println(res_str.c_str());
			client.publish(reply_id, res_str.c_str());      
		} else {
			Serial.println("NO sqe MATCH");
		}
	}
}

void reconnect() {
	while (!client.connected()) {
		String clientId = "ESP32Client-";
		clientId += String(random(0xffff), HEX);
		//mqtt_secret
		// if (client.connect(clientId.c_str(), mqtt_user.c_str(), mqtt_password.c_str())) {
		if (client.connect(clientId.c_str())) {
			Serial.println("connected");
			client.subscribe("/lights/kitchen");
		} else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			delay(5000);
		}
	}
}

void setup() {
	pinMode(output_pin, OUTPUT);
	Serial.begin(115200);

	pixels.begin();
	for(int i=0;i<255;i++) {
		set_strip_color(0,0,0,i);
	}
	Serial.print("on");
	for(int i=255;i>=0;i--) {
		set_strip_color(0,0,0,i);
	}
	Serial.print("off");

	setup_wifi();
	client.setServer(mqtt_server, 1883);
	client.setCallback(callback);
}

void loop() {
	if (!client.connected()) {
		reconnect();
	}
	client.loop();
}