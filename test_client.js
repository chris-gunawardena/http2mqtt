var mqtt = require('mqtt')
var client  = mqtt.connect('mqtt://mqtt.chris.gunawardena.id.au')
 
client.on('connect', function () {
  client.subscribe('/lights/neopixel');
})
 
client.on('message', function (topic, message) {
  console.log(message.toString())
});
