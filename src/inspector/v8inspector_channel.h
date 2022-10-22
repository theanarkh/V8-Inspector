#ifndef V8INSPECTORCHANNELIMP_H
#define V8INSPECTORCHANNELIMP_H

#include <functional>
#include <v8.h>
#include <v8-inspector.h>
#include <list>
#include <arpa/inet.h>

#include "src/inspector/protocol/Forward.h"

using v8_inspector::StringBuffer;
using v8_inspector::StringView;
using v8_inspector::V8Inspector;
using v8_inspector::V8InspectorClient;

class V8InspectorChannelImp final: public v8_inspector::V8Inspector::Channel, public v8_inspector::protocol::FrontendChannel
{
public:
  V8InspectorChannelImp(v8::Isolate* isolate);

  void sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message) override;

  void sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message) override;

  void flushProtocolNotifications() override { }

  void SendProtocolResponse(int callId,
                            std::unique_ptr<v8_inspector::protocol::Serializable> message) {
                               printf("hello");
                            }
  void SendProtocolNotification(
      std::unique_ptr<v8_inspector::protocol::Serializable> message) {
        printf("hello");
  }

  void FallThrough(int call_id,
                           v8_crdtp::span<uint8_t> method,
                           v8_crdtp::span<uint8_t> message) override {}
  void FlushProtocolNotifications() {}
  void send(const char* buf, int size);
private:
  v8::Isolate* isolate_;
  int client_fd_;
  struct sockaddr_in server_addr_;
};

#endif // V8INSPECTORCHANNELIMP_H
