#include "buffers.h"
#include "fmt/printf.h"

using namespace mhttplib;
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
  return 0;
}
