
var mqtt_server = new require('mosca').Server({ port: 1883 });
mqtt_server.on('ready', function setup() {
  console.log('mqtt_server server is up and running')
});
mqtt_server.on('clientConnected', function(client) {
    console.log('client connected', client.id);
});
mqtt_server.on('published', function(packet, client) {
  console.log('Published', packet.payload);
});

var http_server = require('express')();
http_server.use(require("body-parser").text({type: ()=>true}));
http_server.all('*', function(req, res){
    mqtt_server.publish({
        topic: req.path,
        payload: req.body,
        qos: 0, // 0, 1, or 2
        retain: false // or true
    }, function() {
        res.send('OK')
    })
});
http_server.listen(3000);
console.log('http listening on port 3000');
