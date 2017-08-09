#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


int output_pin = 5;
const char* ssid = "FASTWEB-1-D2700B";
const char* password = "978C3B413C";
const char* mqtt_server = "mqtt.chris.gunawardena.id.au";

WiFiClient espClient;
PubSubClient client(espClient);
const size_t bufferSize = 5*JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(0) + 3*JSON_OBJECT_SIZE(1) + 9*JSON_OBJECT_SIZE(2) + 6*JSON_OBJECT_SIZE(3) + 2*JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(7) + JSON_OBJECT_SIZE(10) + 1330;
DynamicJsonBuffer jsonBuffer(bufferSize);

char* string2char(String command){
    if(command.length()!=0){
        char *p = const_cast<char*>(command.c_str());
        return p;
    }
}

bool get_light_state(const char* json) {
  JsonObject& root = jsonBuffer.parseObject(json);
  JsonObject& result = root["result"];
  return strcmp(result["parameters"]["light-state"], "on") == 0;
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


  if (strcmp(topic, "/lights") == 0) {
    String returnTopicStr = String(topic) + String("/return");
    if (get_light_state((char *) payload)) {
      digitalWrite(output_pin, HIGH);
      client.publish(string2char(returnTopicStr), "ON");
      Serial.println("ON");
    } else {
      digitalWrite(output_pin, LOW);
      client.publish(string2char(returnTopicStr), "OFF");
      Serial.println("OFF");
    }
  }


}

void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("/lights");
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

