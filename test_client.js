var mqtt = require('mqtt')
var client  = mqtt.connect('mqtt://37.139.12.249')
 
client.on('connect', function () {
  client.subscribe('/lights/neopixel');
})
 
client.on('message', function (topic, message) {
  console.log(message.toString())
});
