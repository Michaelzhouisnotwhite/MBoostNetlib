#include <TcpServer.h>
#include <toy/type.h>
#include <boost/asio.hpp>
#include <cstdio>
#include "HttpServer.h"
#include "fmt/core.h"
int main(int argc, char* argv[]) {
  boost::asio::io_context io;

  auto threads = Vec<std::thread>();
  auto server = HttpAsyncServer(io, "0.0.0.0", 8888);
  server.SetHttpHandler([](const HttpHandlerReq& req)
                            -> HttpHandlerResp {
    mnet::HttpHeader header;
    header.SetContentType("text/txt");
    header.SetHeader("username", "michael");
    return mnet::HttpBaseResponse::Create()
        ->Header(header)
        ->Body(fmt::format("your address is: {}:{}", req->addr.to_string(), req->port))
        ->StatusCode(200);
  });

  mnet::ThreadPrinter::Instance();
  for (int i = 0; i < 4; i++) {
    threads.emplace_back([&io, &server]() {
      server.Start();
      io.run();
    });
  }
  for (auto& thr : threads) {
    thr.join();
  }
  // io.run();
  return 0;
}
