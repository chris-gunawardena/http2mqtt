#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


int output_pin = 5;
const char* ssid = "FASTWEB-1-D2700B";
const char* password = "978C3B413C";
const char* mqtt_server = "mqtt.chris.gunawardena.id.au";
// const char* mqtt_user = "mqtt_user";
// const char* mqtt_password = "mqtt_secret";

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
    JsonObject& root = jsonBuffer.parseObject((char *) payload);
    const char* reply_id = root["reply_id"]; // "/reply/id_8106"
    const char* light_state = root["body"]["result"]["parameters"]["light-state"]; // "on"
    Serial.println(light_state);

    if (strcmp(light_state, "on") == 0) {
      digitalWrite(output_pin, HIGH);
      client.publish(reply_id, "{\"speech\": \"Switching lights on\", \"displayText\": \"Switching lights on\", \"data\": {}, \"contextOut\": [], \"source\": \"mqtt\" }");
    } else {
      digitalWrite(output_pin, LOW);
      client.publish(reply_id, "{\"speech\": \"Switching lights off\", \"displayText\": \"Switching lights off\", \"data\": {}, \"contextOut\": [], \"source\": \"mqtt\" }");
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

