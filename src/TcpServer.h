#ifndef SRC_TCPSERVER
#define SRC_TCPSERVER
#include <toy/type.h>
#include <boost/asio.hpp>

class TcpAsyncServerBase {
public:
  virtual ~TcpAsyncServerBase() = default;
  explicit TcpAsyncServerBase(boost::asio::io_context& ioc, const String& host, u32 port);
  void Start();

protected:
  virtual void HandleAccept(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                            boost::system::error_code ec) = 0;

private:
  boost::asio::io_context& ioc_;
  using tcp = boost::asio::ip::tcp;
  tcp::acceptor accepter_;
  void StartAccept();
};

#endif /* SRC_TCPSERVER */
