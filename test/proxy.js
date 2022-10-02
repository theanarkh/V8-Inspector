const { WebSocketServer } = require('ws');
const dgram = require('dgram');

const udpClient = dgram.createSocket('udp4');
udpClient.bind(5555);

const wsServer = new WebSocketServer({
    port: 9229,
});

wsServer.on('connection', (socket) => {
  udpClient.on('message', (data) => {
    socket.send(data.toString());
  });
  socket.on('message', (data) => {
    udpClient.send(data, 8888);
  });
});
