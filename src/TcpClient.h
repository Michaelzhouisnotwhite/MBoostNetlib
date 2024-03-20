#ifndef SRC_TCPCLIENT
#define SRC_TCPCLIENT
#include <toy/type.h>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <memory>
#include <utility>
#include "buffers.h"

class TcpInformation {
public:
  String host;
  int port;
  int sockfd;
};

class TcpSession : std::enable_shared_from_this<TcpSession> {
public:
  explicit TcpSession();
  void Create(String host, int port);
  void SetConnectionHandler(std::function<void(const TcpInformation&)> conn_handler);
  void SetMsgHandler(std::function<Vec<char>(const Vec<char>&)> msg_handler);
  void Start();
  inline void Send(Vec<char> msg) {
    return Send(std::move(msg), nullptr);
  }
  inline void Send(const String& msg) {
    return Send(msg, nullptr);
  }
  void Send(Vec<char> msg, std::function<void()> cb);
  void Send(const String& msg, std::function<void()> cb);
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
  void StartSend();
  void StartRecv();
  mnet::VecBuffer<char> read_buf_;
  std::thread io_thread_;

  VecDeque<std::pair<Vec<char>, decltype(after_msg_handler_)>> send_buf_;
};

#endif /* SRC_TCPCLIENT */
