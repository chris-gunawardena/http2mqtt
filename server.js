
var mqtt_server = new require('mosca').Server({ port: 1883 });
mqtt_server.on('ready', function setup() {
  console.log('mqtt_server server is up and running')
});
mqtt_server.on('clientConnected', function(client) {
    console.log('client connected', client.id);
});
var responses = {};
mqtt_server.on('published', function(packet, client) {
  console.log('Published', packet.topic, packet.payload);
  if(responses[packet.topic]) {
      responses[packet.topic].setHeader('Content-type', 'application/json');
      responses[packet.topic].send(packet.payload);
      responses[packet.topic] = null;
  }
});

var express_app = require('express')();
var https = require('https');
var fs = require('fs');
var https_options = {
    key: fs.readFileSync('/etc/letsencrypt/live/' + process.env.SSL_HOST + '/privkey.pem'),
    cert: fs.readFileSync('/etc/letsencrypt/live/' + process.env.SSL_HOST + '/fullchain.pem'),
    ca: fs.readFileSync('/etc/letsencrypt/live/' + process.env.SSL_HOST + '/chain.pem')
};

express_app.use(require("body-parser").text({type: ()=>true}));
express_app.all('*', function(req, res){
    console.log(req.path, req.body);
    // store response until reply from mqtt client
    let reply_id = '/reply/id_' + Math.ceil(Math.random()*10000);
    responses[reply_id] = res;
    mqtt_server.publish({
        topic: req.path,
        payload: {
            body: req.body,
            reply_id: reply_id
        },
        qos: 0, // 0, 1, or 2
        retain: false // or true
    });
});
https.createServer(https_options, express_app).listen(443);

