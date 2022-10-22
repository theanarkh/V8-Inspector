#include <functional>
#include "v8inspector_channel.h"
#include "utils.h"

V8InspectorChannelImp::V8InspectorChannelImp(v8::Isolate *isolate) {
    isolate_ = isolate;
    client_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family       = AF_INET;
    client_addr.sin_port         = htons(6666);
    client_addr.sin_addr.s_addr  = htonl(INADDR_ANY);
    if (bind(client_fd_, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0) { 
        perror("bind address error"); 
    }
    set_buf_size(client_fd_);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr_.sin_family       = AF_INET;
    server_addr_.sin_port         = htons(8888);
    server_addr_.sin_addr.s_addr  = htonl(INADDR_ANY);
}

void V8InspectorChannelImp::sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message) {
    const std::string response = convertToString(isolate_, message->string());
    send(response.c_str(), response.length());
}

void V8InspectorChannelImp::sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message) {
    const std::string notification = convertToString(isolate_, message->string());
    send(notification.c_str(), notification.length());
}

void V8InspectorChannelImp::send(const char* buf, int size) {
  int offset = 0;
  while(offset != size) {
    int left = size - offset;
    int ret = sendto(client_fd_, buf + offset, left > 10000 ? 10000 : left, 0, (struct sockaddr*)&server_addr_, sizeof(server_addr_)); 
    offset += ret;
    printf("received %d, sended %d, offset %d\n", size, ret, offset);
  }
}