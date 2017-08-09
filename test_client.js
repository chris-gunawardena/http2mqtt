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
  console.log('ok');