#include "v8inspector_client.h"

V8InspectorClientImpl::V8InspectorClientImpl(const std::unique_ptr<v8::Platform> &platform, const v8::Local<v8::Context> &context) {
    platform_ = platform.get();
    context_ = context;
    isolate_ = context_->GetIsolate();
    channel_.reset(new V8InspectorChannelImp(isolate_));
    inspector_ = v8_inspector::V8Inspector::create(isolate_, this);
    session_ = inspector_->connect(kContextGroupId, channel_.get(), v8_inspector::StringView());
    std::string contextName = "NoInspector";
    v8_inspector::V8ContextInfo v8info(context, kContextGroupId, convertToStringView(contextName));
    inspector_->contextCreated(v8info);
    terminated_ = true;
    run_nested_loop_ = false;
}

void V8InspectorClientImpl::dispatchProtocolMessage() {
    std::vector<std::string>::iterator it;
    std::vector<std::string> queues;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        requests_.swap(queues);
    }
    for(it = queues.begin(); it != queues.end(); it++)
    {
        session_->dispatchProtocolMessage(convertToStringView(*it));
        v8::Local<v8::Object> jsonObject = parseJson(context_, *it);
        if (!jsonObject.IsEmpty()) {
            std::string method = getPropertyFromJson(context_->GetIsolate(), jsonObject, "method");
            if (method == "Runtime.runIfWaitingForDebugger") {
                schedulePauseOnNextStatement(convertToStringView("test"));
                waitFrontendMessageOnPause();
            }
        }
    }
}

void V8InspectorClientImpl::runMessageLoopOnPause(int contextGroupId) {
    if (run_nested_loop_) {
        return;
    }
    std::cout<<"runMessageLoopOnPause"<<std::endl;
    terminated_ = false;
    run_nested_loop_ = true;
    while (!terminated_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_variable_.wait(lock);
        }
        dispatchProtocolMessage();
    }
    terminated_ = true;
    run_nested_loop_ = false;
}

void V8InspectorClientImpl::quitMessageLoopOnPause() {
    terminated_ = true;
}

v8::Local<v8::Context> V8InspectorClientImpl::ensureDefaultContextInGroup(int contextGroupId) {
    return context_;
}

void V8InspectorClientImpl::schedulePauseOnNextStatement(const v8_inspector::StringView &reason) {
    session_->schedulePauseOnNextStatement(reason, reason);
}

void V8InspectorClientImpl::waitFrontendMessageOnPause() {
    terminated_ = false;
}

void V8InspectorClientImpl::onMessage(char *buf, size_t count) {
    std::lock_guard<std::mutex> guard(mutex_);
    requests_.push_back(std::string(buf, 0, count));
    if (requests_.size() == 1) {
        isolate_->RequestInterrupt([](v8::Isolate* isolate, void* data) {
            V8InspectorClientImpl *client = static_cast<V8InspectorClientImpl *>(data);
            client->dispatchProtocolMessage();
        }, this); 
    }
    condition_variable_.notify_one();
}