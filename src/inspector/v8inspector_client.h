
#ifndef V8INSPECTORCLIENTIMPL_H
#define V8INSPECTORCLIENTIMPL_H

#include <iostream>
#include <v8.h>
#include <v8-inspector.h>
#include <libplatform/libplatform.h>
#include "v8inspector_channel.h"
#include "utils.h"

class V8InspectorClientImpl final: public v8_inspector::V8InspectorClient {
public:
  V8InspectorClientImpl(const std::unique_ptr<v8::Platform> &platform, const v8::Local<v8::Context>& context);
  ~V8InspectorClientImpl() {}
  void dispatchProtocolMessage();

  void runMessageLoopOnPause(int contextGroupId) override;

  void quitMessageLoopOnPause() override;

  void schedulePauseOnNextStatement(const v8_inspector::StringView &reason);

  void waitFrontendMessageOnPause();

  void onMessage(char *buf, size_t count);
  
private:
  v8::Local<v8::Context> ensureDefaultContextInGroup(int contextGroupId) override;

  static const int kContextGroupId = 1;
  v8::Platform* platform_;
  std::unique_ptr<v8_inspector::V8Inspector> inspector_;
  std::unique_ptr<v8_inspector::V8InspectorSession> session_;
  std::unique_ptr<V8InspectorChannelImp> channel_;
  v8::Isolate* isolate_;
  v8::Handle<v8::Context> context_;
  uint8_t terminated_ = 0;
  bool run_nested_loop_ = false;
  std::condition_variable condition_variable_;
  std::vector<std::string> requests_;
  std::mutex mutex_;
};

#endif // V8INSPECTORCLIENTIMPL_H
