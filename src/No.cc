#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <thread> 
#include <deque>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include "libplatform/libplatform.h"
#include "v8.h"
#include "inspector/v8inspector_client.h"

using namespace v8;

void worker(V8InspectorClientImpl* client) {
    struct sockaddr_in server_addr;
    int connfd;
    struct sockaddr_in clent_addr; 
    socklen_t len = sizeof(clent_addr);
    size_t count;
    char* buf = new char[BUF_LEN];
    int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0) { 
        perror("create socket error"); 
        goto EXIT;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family       = AF_INET;
    server_addr.sin_port         = htons(8888);
    server_addr.sin_addr.s_addr  = htonl(INADDR_ANY);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) { 
        perror("bind address error"); 
        goto EXIT;
    }
    set_buf_size(server_fd);
    while(1)
    {
        struct sockaddr_in server; 
        socklen_t server_len = sizeof(server);
        memset(&server, 0, sizeof(server));
        server.sin_family       = AF_INET;
        server.sin_port         = htons(5555);
        server.sin_addr.s_addr  = htonl(INADDR_ANY);

        memset(buf, 0, BUF_LEN);
        count = recvfrom(server_fd, buf, BUF_LEN, 0, (struct sockaddr*)&clent_addr, &len);
        if(count == -1)
        {
            continue;
        }
        int client_port = htons(clent_addr.sin_port);
        // From V8 inspector
        if (client_port == 6666) {
            sendto(server_fd, buf, count, 0, (struct sockaddr*)&server, server_len); 
        } else { 
            // From Inspector client, such as Chrome Dev Tools
            client->onMessage(buf, count);
        }
    }
    close(server_fd);
EXIT:
    delete[] buf;
    return;
}

int main(int argc, char* argv[]) {

  std::thread t;
  setvbuf(stdout, nullptr, _IONBF, 0);
  setvbuf(stderr, nullptr, _IONBF, 0);
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<Platform> platform = platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();
  Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
  Isolate* isolate = Isolate::New(create_params);
  Isolate::Scope isolate_scope(isolate);
  HandleScope handle_scope(isolate);
  Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
  Local<Context> context = Context::New(isolate, nullptr, global);
  Context::Scope context_scope(context);
  std::unique_ptr<V8InspectorClientImpl> client = std::make_unique<V8InspectorClientImpl>(platform, context);
  // 打开文件
  int fd = open(argv[1], 0, O_RDONLY);
  if (fd == -1) {
    std::cout<<"file not found";
    return errno;
  }
  t = std::thread(worker, client.get());
  {
    struct stat info;
    // 取得文件信息
    fstat(fd, &info);
    // 分配内存保存文件内容
    char *ptr = (char *)malloc(info.st_size + 1);
    // ptr[info.st_size] = '\0';
    read(fd, (void *)ptr, info.st_size);
    // 要执行的js代码
    Local<String> source = String::NewFromUtf8(isolate, ptr,
                        NewStringType::kNormal,
                        info.st_size).ToLocalChecked();
    ScriptOrigin origin(String::NewFromUtf8(isolate, "V8-Inspector", NewStringType::kNormal, strlen("V8-Inspector")).ToLocalChecked());

    // 编译
    Local<Script> script = Script::Compile(context, source, &origin).ToLocalChecked();
    // 解析完应该没用了，释放内存
    free(ptr);
    // 执行
    Local<Value> result = script->Run(context).ToLocalChecked();
    
  }
  
  t.join();
  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  delete create_params.array_buffer_allocator;
  return 0;
}
