#ifndef HTTPSERVER_H
#define HTTPSERVER_H
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include "TcpServer.h"
#include "buffers.h"
class HttpPayload {
public:
  String Body();
};

class HttpAsyncClient : public std::enable_shared_from_this<HttpAsyncClient> {
  friend class HttpAsyncServer;
  using tcp = boost::asio::ip::tcp;

public:
  explicit HttpAsyncClient(std::shared_ptr<tcp::socket> socket);

private:
  void StartRecv();
  void HandleReadSome(const boost::system::error_code& error, std::size_t bytes_transferred);
  u64 buf_size_ = 8192;
  std::shared_ptr<tcp::socket> socket_;
  mhttplib::VecBuffer<char> recv_buffer_;
  boost::beast::http::request_parser<boost::beast::http::string_body> req_;
  VecDeque<char> send_buf_;
};

class HttpAsyncServer : public TcpAsyncServerBase {
public:
  HttpAsyncServer(boost::asio::io_context& io, const String& host, u32 port);

protected:
  using tcp = boost::asio::ip::tcp;
  void HandleAccept(std::shared_ptr<tcp::socket> socket, boost::system::error_code ec) override;

private:
  VecDeque<std::shared_ptr<HttpAsyncClient>> clients_;
};

#endif  // HTTPSERVER_H