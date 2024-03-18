#include "TcpClient.h"

int main(int argc, char* argv[]) {
  auto session = TcpSession();
  session.Create("0.0.0.0", 8089);
  session.SetMsgHandler([&session](const Vec<char>& msg) {
    fmt::println("{}", mnet::VecBuf2String(msg));
    session.SetAfterMsgHandler([&session]() {
      session.Close();
    });
    return mnet::MakeVecBuf("abc");
  });
  session.Start();
  session.Send("Hello From Client");
  session.Wait();
  return 0;
}