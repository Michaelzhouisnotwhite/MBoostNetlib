#include <HttpUtils.h>

#include "buffers.h"
#include "fmt/printf.h"

using namespace mhttplib;

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
int main() {
  VecBuffer<char> buf(100, 0);
  auto b = buf.begin();
  fmt::println("{}", *b);

  String name = "Michael";
  std::memcpy(buf.data(), name.data(), name.length());

  std::puts(buf.PrettyStr().c_str());
  fmt::println("buf remain size: {}", buf.RemainByteSize());
  auto front = buf.PopFront(name.size());
  for (const auto& value : front) {
    fmt::print("{}", value);
  }
  std::puts("");

  std::puts(buf.PrettyStr().c_str());
  fmt::println("buf remain size: {}", buf.RemainByteSize());

  HttpRequestParser parser;
  int ec;
  auto size = parser.Put({header_post.begin(), header_post.end()}, ec);
  auto remain_req = String(header_post.begin() + static_cast<long int>(size), header_post.end());
  size = parser.Put({remain_req.begin(), remain_req.end()}, ec);
  return 0;
}
