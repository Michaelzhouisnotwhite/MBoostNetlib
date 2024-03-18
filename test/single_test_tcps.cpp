#include "TcpServer.h"
std::string header_post =
    "POST /post-test HTTP/1.1\r\n"
    "Host: 127.0.0.1:8080\r\n"
    "User-Agent: Apifox/1.0.0 (https://apifox.com)\r\n"
    "Content-Type: application/json\r\n"
    "Accept: */*\r\n"
    "Host: 127.0.0.1:8080\r\n"
    "Connection: keep-alive\r\n"
    "Content-Length: 67\r\n"
    "\r\n"
    "{\"name\":\"Foo\",\"description\":\"The pretender\",\"price\":42.0,\"tax\":3.2}";
int main(int argc, char* argv[]) {
  boost::asio::io_context io;
  auto threads = Vec<std::thread>();
  auto server = TcpAsyncServer(io, "0.0.0.0", 8089);
  server.SetHandler([](const Vec<char>& input) -> Vec<char> {
    
    fmt::println("{}", mnet::VecBuf2String(input));
    return mnet::MakeVecBuf(header_post);
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
  return 0;
}
