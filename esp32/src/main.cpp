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
int red = 0, green = 0, blue = 0, brightness = 0;
// const char* mqtt_user = "mqtt_user";
// const char* mqtt_password = "mqtt_secret";

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGBW + NEO_KHZ800);
bool on = false;

const char* sync_res_template = "{\"requestId\":\"xxxx\",\"payload\":{\"devices\":[{\"id\":\"456\",\"type\":\"action.devices.types.LIGHT\",\"traits\":[\"action.devices.traits.OnOff\",\"action.devices.traits.Brightness\",\"action.devices.traits.ColorSpectrum\"],\"name\":{\"name\":\"kitchen light\"},\"willReportState\":false}]}}";
const char* query_res_template = "{\"requestId\":\"yyyy\",\"payload\":{\"devices\":{\"456\":{\"online\":true}}}}";
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
    DynamicJsonBuffer req_buffer(2048);    
    JsonObject& req_root = req_buffer.parseObject((char *) payload);
    const char* reply_id = req_root["reply_id"]; // "/reply/id_8106"
    const char* requestId = req_root["body"]["requestId"];
    const char* response;    
    const char* intent = req_root["body"]["inputs"][0]["intent"]; // "action.devices.SYNC"
    Serial.println("00000000000");
    Serial.println(intent);
    Serial.println("1111111111");
    
    if (strcmp(intent, "action.devices.SYNC") == 0) {
      Serial.println("2222");
      response = sync_res_template;
    } else if (strcmp(intent, "action.devices.QUERY") == 0) {
      Serial.println("3333");
      response = query_res_template;
    } else if (strcmp(intent, "action.devices.EXECUTE") == 0) {
      Serial.println("4444");
      const char* command = req_root["body"]["inputs"][0]["payload"]["commands"][0]["execution"][0]["command"]; // "action.devices.commands.OnOff"
      Serial.println("5555");
      if (strcmp(command, "action.devices.commands.OnOff") == 0) {
        Serial.println("6666");
        on = req_root["body"]["inputs"][0]["payload"]["commands"][0]["execution"][0]["params"]["on"];
        if(on) {
          digitalWrite(output_pin, HIGH);
          brightness = 255;
        } else {
          digitalWrite(output_pin, LOW);
          brightness = 0;
        }
      } else if (strcmp(command, "action.devices.commands.ColorAbsolute") == 0) {
        Serial.println("7777");
        // uint32_t rgbHex = req_root["body"]["inputs"][0]["payload"]["commands"][0]["execution"][0]["params"]["color"]["spectrumRGB"];
        // Serial.println(rgbHex);
        // red = (rgbHex >> 16) & 0xFF;
        // green = (rgbHex >> 8) & 0xFF;
        // blue = rgbHex & 0xFF;
      }
      Serial.println("----");
      
      set_strip_color(red,green,blue,brightness);
      response = exec_res_template;
    }
    Serial.println("88888");
    
    DynamicJsonBuffer res_buffer(1024);      
    JsonObject& res_root = res_buffer.parseObject(response);
    res_root["requestId"] = requestId;
    String res_str;
    res_root.printTo(res_str);
    Serial.println(res_str.c_str());
    client.publish(reply_id, res_str.c_str());      
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