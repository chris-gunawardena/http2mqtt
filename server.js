var request = require('request');
var express_app = require('express')();
var basicAuth = require('express-basic-auth');
var https = require('https');
var fs = require('fs');
var mqtt_server = new require('mosca').Server({ port: 1883 });


var responses = {};
mqtt_server.on('published', function(packet, client) {
  console.log('Published', packet.topic, packet.payload);
  // if there is a response waiting for this topic
  if (responses[packet.topic]) {
    responses[packet.topic].setHeader('Content-type', 'application/json');
    responses[packet.topic].send(packet.payload);
    responses[packet.topic] = null;
  } else if(packet.topic.startsWith('/request') && !packet.topic.startsWith('/request/reply')) {
    //make a http request
    console.log('payload', packet.payload);
    var options = JSON.parse(packet.payload);
    request(options, (error, response, body) => {
        if (error) {
            return console.error('upload failed:', error);
        }
        console.log('Upload successful!  Server responded with:', body);
        if (options.reply_id) {
            mqtt_server.publish({
                topic: options.reply_id,
                payload: JSON.stringify(body),
                qos: 0, // 0, 1, or 2
                retain: false // or true
            });
        }
    })
  }
});
mqtt_server.on('ready', function setup() {
  console.log('mqtt_server server is up and running');
//   mqtt_server.authenticate = function(client, username, password, callback) {
//     var authorized = (username === 'mqtt_user' && password.toString() === 'mqtt_secret');
//     if (authorized) client.user = username;
//     callback(null, authorized);
//   };
});
mqtt_server.on('clientConnected', function(client) {
    console.log('client connected', client.id);
});

// express_app.use(basicAuth({
//     users: { 'rest_user': 'rest_secret' }
// }));
express_app.use(require("body-parser").text({type: ()=>true}));
express_app.all('*', function(req, res){
    console.log(req.path, req.body);
    // store response until reply from mqtt client
    var reply_id = '/reply/id_' + Math.ceil(Math.random()*10000);
    responses[reply_id] = res;
    mqtt_server.publish({
        topic: req.path,
        payload: JSON.stringify({
            body: JSON.parse(req.body),
            reply_id: reply_id
        }),
        qos: 0, // 0, 1, or 2
        retain: false // or true
    });
});
if (process.env.SSL_HOST) {
    https.createServer({
        key: fs.readFileSync('/etc/letsencrypt/live/' + process.env.SSL_HOST + '/privkey.pem'),
        cert: fs.readFileSync('/etc/letsencrypt/live/' + process.env.SSL_HOST + '/fullchain.pem'),
        ca: fs.readFileSync('/etc/letsencrypt/live/' + process.env.SSL_HOST + '/chain.pem')
    }, express_app).listen(443);
} else {
    console.log('please setup letsencrypt');
}

