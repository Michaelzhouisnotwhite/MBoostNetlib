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
  explicit TcpSession( boost::asio::io_context& io);
  void Create(String host, int port);
  void SetConnectionHandler(std::function<void(const TcpInformation&)> conn_handler);
  void SetMsgHandler(std::function<Vec<char>(const Vec<char>&)> msg_handler);
  void Start();

private:
  void HandleConn(boost::system::error_code ec);
  void HandleRead(boost::system::error_code ec, size_t bytes_len);
  String host_{};
  boost::asio::ip::tcp::socket sockfd_;
  std::function<void(const TcpInformation&)> conn_handler_;
  int port_{};
  std::function<Vec<char>(const Vec<char>&)> msg_handler_;
  boost::asio::io_context& io_;

  mnet::VecBuffer<char> read_buf_;
};

#endif /* SRC_TCPCLIENT */
