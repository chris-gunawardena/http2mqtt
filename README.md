# http2mqtt
## Nodejs https->mqtt->https and mqtt->https->mqtt Gateway

### https -> mqtt -> https


[![HTTPS to MQTT](https://media.giphy.com/media/XyPqaI0ImIPfi/giphy.gif)](https://www.youtube.com/watch?v=ZfAJ-2c3QO8 "Everything Is AWESOME")

#### JS client example

#### ESP32 client example

### mqtt -> https -> mqtt

#### JS client example

#### ESP32 client example

### Setup steps
1. Install docker. https://certbot.eff.org/#ubuntuxenial-other
2. User certbot to create SSL certs on the host machine. https://certbot.eff.org/#ubuntuxenial-other
3. Clone this repo.
4. Update the SSL_HOST env var here: https://github.com/chris-gunawardena/http2mqtt.js/blob/master/docker-compose.yml#L8
5. Run `docker-compose up -d` from the root folder of this repo.


