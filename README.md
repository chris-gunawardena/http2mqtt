# Nodejs https->mqtt->https and mqtt->https->mqtt Gateway
## Allows HTTPS web services connect to IoT devices via MQTT and IoT devices make HTTPS calls via MQTT

### https -> mqtt -> https (Allows HTTPS web services connect to IoT devices via MQTT)
[![HTTPS to MQTT](https://media.giphy.com/media/XyPqaI0ImIPfi/giphy.gif)](https://www.youtube.com/watch?v=ZfAJ-2c3QO8 "youtube video")

#### JS client example
```js
var mqtt = require('mqtt');
var client  = mqtt.connect('mqtt://mqtt.chris.gunawardena.id.au');
 
client.on('connect', function () {
  client.subscribe('/lights');
})
 
client.on('message', function (topic, message) {
  console.log(message.toString());
  var msg_obj = JSON.parse(message.toString());
  client.publish(msg_obj.reply_id, 'xoxo');
});
```

#### ESP32 (C++) client example
https://github.com/chris-gunawardena/http2mqtt.js/blob/master/esp32/src/main.cpp
```cpp
void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, "/lights") == 0) {
    JsonObject& root = jsonBuffer.parseObject((char *) payload);
    const char* reply_id = root["reply_id"]; // "/reply/id_8106"
    const char* light_state = root["body"]["result"]["parameters"]["light-state"]; // "on"

    if (strcmp(light_state, "on") == 0) {
      client.publish(reply_id, "{\"speech\": \"Switching lights on\", \"displayText\": \"Switching lights on\", \"data\": {}, \"contextOut\": [], \"source\": \"mqtt\" }");
    } else {
      client.publish(reply_id, "{\"speech\": \"Switching lights off\", \"displayText\": \"Switching lights off\", \"data\": {}, \"contextOut\": [], \"source\": \"mqtt\" }");
    }    
  }
}
```

### mqtt -> https -> mqtt (Allows IoT devices make HTTPS calls via MQTT)
[![HTTPS to MQTT](https://media.giphy.com/media/8lpOGBGJ4BRE4/giphy.gif)](https://youtu.be/XBuaXGn8jrI "youtube video")

#### ESP32 (C++) client example
https://github.com/chris-gunawardena/IoT-H2O/blob/master/esp32/src/main.cpp
```cpp
    while (!pubSubClient.connected()) {
      String clientId = "esp32client" + String(random(0xffff), HEX);
      reply_id = "/request/reply/" + clientId;
      if (pubSubClient.connect(clientId.c_str())) {
        Serial.println("mqtt connected");
        JsonObject& root = jsonBuffer.parseObject(request_json_template);
        root["reply_id"] = reply_id;
        String request;
        root.printTo(request);
        pubSubClient.subscribe(reply_id.c_str());
        pubSubClient.publish("/request", request.c_str());
      }
```

### Setup steps
1. Install docker. https://certbot.eff.org/#ubuntuxenial-other
2. User certbot to create SSL certs on the host machine. https://certbot.eff.org/#ubuntuxenial-other
3. Clone this repo.
4. Update the SSL_HOST env var here: https://github.com/chris-gunawardena/http2mqtt.js/blob/master/docker-compose.yml#L8
5. Run `docker-compose up -d` from the root folder of this repo.


