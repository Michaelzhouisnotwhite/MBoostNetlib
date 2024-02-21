#include "buffers.h"
#include "fmt/printf.h"

using namespace mhttplib;
int main() {
  VecBuffer<char> buf(100, 'a');
  auto b = buf.begin();
  fmt::println("{}", *b);

  String name = "Michael";
  std::memcpy(buf.data(), name.data(), name.length());

  for (const auto& value : buf.Front(name.size())) {
    fmt::print("{}", value);
  }
  std::puts("");

  auto front = buf.PopFront(name.size());
  for (const auto& value : front) {
    fmt::print("{}", value);
  }
  std::puts("");

  for (const auto& value : buf) {
    fmt::print("{}", value);
  }
  std::puts("");
  fmt::println("buf remain size: {}", buf.size());
  return 0;
}
