#include "TcpClient.h"

int main(int argc, char* argv[]) {
  auto session = TcpSession();
  session.Create("0.0.0.0", 8089);
  session.SetMsgHandler([&session](const Vec<char>& msg) {
    fmt::println("{}", mnet::VecBuf2String(msg));
    session.SetAfterMsgHandler([&session]() {
      session.Close();
    });
    return mnet::MakeVecBuf("Hello From Client2");
  });
  session.Start();
  session.Send("Hello From Client");
  session.Send("Hello From Client3");
  session.Send("Hello From Client4");
  session.Send("Hello From Client5");
  session.Wait();
  return 0;
}