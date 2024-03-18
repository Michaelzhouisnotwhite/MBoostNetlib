#ifndef SRC_TCPCLIENT
#define SRC_TCPCLIENT
#include <toy/type.h>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <memory>
#include "buffers.h"

class TcpInformation {
public:
  String host;
  int port;
  int sockfd;
};

class TcpSession {
public:
  explicit TcpSession();
  void Create(String host, int port);
  void SetConnectionHandler(std::function<void(const TcpInformation&)> conn_handler);
  void SetMsgHandler(std::function<Vec<char>(const Vec<char>&)> msg_handler);
  void Start();
  void Send(Vec<char> msg);
  void Send(const String& msg);
  void Wait();
  void Close();
  void SetAfterMsgHandler(std::function<void()> handler);

private:
  void HandleConn(boost::system::error_code ec);
  void HandleRead(boost::system::error_code ec, size_t bytes_len);
  String host_{};
  std::optional<boost::asio::ip::tcp::socket> sockfd_;
  std::function<void(const TcpInformation&)> conn_handler_;
  int port_{};
  std::function<void()> after_msg_handler_;
  std::function<Vec<char>(const Vec<char>&)> msg_handler_;
  boost::asio::io_context io_;
  void StartRecv();
  mnet::VecBuffer<char> read_buf_;
  std::thread io_thread_;
};

#endif /* SRC_TCPCLIENT */
