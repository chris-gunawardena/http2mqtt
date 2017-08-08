
var mqtt_server = new require('mosca').Server({ port: 1883 });
mqtt_server.on('ready', function setup() {
  console.log('mqtt_server server is up and running')
});
mqtt_server.on('clientConnected', function(client) {
    console.log('client connected', client.id);
});

var express_app = require('express')();
var https = require('https');
var fs = require('fs');
var https_options = {
    key: fs.readFileSync('/etc/letsencrypt/live/' + process.env.SSL_HOST + '/privkey.pem'),
    cert: fs.readFileSync('/etc/letsencrypt/live/' + process.env.SSL_HOST + '/cert.pem')
};
express_app.use(require("body-parser").text({type: ()=>true}));
express_app.all('*', function(req, res){
    console.log(req.path, req.body);
    mqtt_server.publish({
        topic: req.path,
        payload: req.body,
        qos: 0, // 0, 1, or 2
        retain: false // or true
    }, function() {
        res.send('OK')
    })
});

https.createServer(https_options, express_app).listen(443);

