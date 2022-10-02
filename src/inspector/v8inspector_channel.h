#ifndef V8INSPECTORCHANNELIMP_H
#define V8INSPECTORCHANNELIMP_H

#include <functional>
#include <v8.h>
#include <v8-inspector.h>
#include <list>
#include <arpa/inet.h>

class V8InspectorChannelImp final: public v8_inspector::V8Inspector::Channel
{
public:
  V8InspectorChannelImp(v8::Isolate* isolate);

  void sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message) override;

  void sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message) override;

  void flushProtocolNotifications() override;
  
  void send(const char* buf, int size);
private:
  v8::Isolate* isolate_;
  int client_fd_;
  struct sockaddr_in server_addr_;
};

#endif // V8INSPECTORCHANNELIMP_H
