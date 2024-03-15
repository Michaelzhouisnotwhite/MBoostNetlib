#ifndef SRC_TCPCLIENT
#define SRC_TCPCLIENT
#include <toy/type.h>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <memory>
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
  explicit TcpConnection(boost::asio::io_context& io, const String& host, int port);

private:
  String host_;
  boost::asio::ip::tcp::socket sockfd_;
  int port_;
};

class TcpSession {
public:
  TcpSession();
  std::shared_ptr<TcpConnection> CreateClient(const String& host, int port);
private:
  boost::asio::io_context io_;
};

#endif /* SRC_TCPCLIENT */
