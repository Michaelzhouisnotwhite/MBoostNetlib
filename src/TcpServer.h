#ifndef SRC_TCPSERVER
#define SRC_TCPSERVER
#include <toy/type.h>
#include <boost/asio.hpp>

#include "buffers.h"

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

class TcpAsyncClient : public std::enable_shared_from_this<TcpAsyncClient> {
  friend class TcpAsyncServer;
  using tcp = boost::asio::ip::tcp;

public:
  explicit TcpAsyncClient(std::shared_ptr<tcp::socket> client_sock);
  void StartRecv();
  void SetHandleFunc(std::function<Vec<char>(Vec<char>)> callback);

private:
  void HandleReadSome(const boost::system::error_code& error, std::size_t bytes_transferred);
  std::function<Vec<char>(Vec<char>)> call_back_;
  std::shared_ptr<tcp::socket> socket_;
  mnet::VecBuffer<char> recv_buffer_;
};

class TcpAsyncServer : public TcpAsyncServerBase {
public:
  explicit TcpAsyncServer(boost::asio::io_context& ioc, const String& host, u32 port);
  void SetHandler(decltype(TcpAsyncClient::call_back_) callback);

private:
  void HandleAccept(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                    boost::system::error_code ec) override;

  decltype(TcpAsyncClient::call_back_) call_back_;
};

#endif /* SRC_TCPSERVER */
