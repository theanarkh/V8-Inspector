const { WebSocketServer } = require('ws');
const dgram = require('dgram');

const udpClient = dgram.createSocket('udp4');
udpClient.bind(5555, () => {
  udpClient.setRecvBufferSize(1*1024*1024);
  udpClient.setSendBufferSize(1*1024*1024);
});

const wsServer = new WebSocketServer({
    port: 9229,
});

wsServer.on('connection', (socket) => {
  let chunk;
  udpClient.on('message', (data) => {
    if (Buffer.byteLength(data) === 10000) {
      chunk = chunk ? Buffer.concat([chunk, data]) : data;
    } else {
      socket.send(chunk ? Buffer.concat([chunk, data]).toString() : data.toString());
      chunk = null;
    }
  });
  socket.on('message', (data) => {
    udpClient.send(data, 8888);
  });
});
