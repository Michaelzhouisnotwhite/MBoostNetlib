#ifndef HTTPSERVER_H
#define HTTPSERVER_H
#include "HttpUtils.h"

#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include "TcpServer.h"
#include "buffers.h"

class HttpAsyncClient : public std::enable_shared_from_this<HttpAsyncClient> {
  friend class HttpAsyncServer;
  using tcp = boost::asio::ip::tcp;

public:
  explicit HttpAsyncClient(std::shared_ptr<tcp::socket> socket);

private:
  void StartRecv();
  void HandleReadSome(const boost::system::error_code& error, std::size_t bytes_transferred);
  u64 buf_size_ = 8000;
  std::shared_ptr<tcp::socket> socket_;
  mhttplib::VecBuffer<char> recv_buffer_;
  mhttplib::HttpRequestParser req_;
  std::function<std::shared_ptr<mhttplib::HttpBaseResponse>(
      std::shared_ptr<mhttplib::HttpRequest> req)>
      on_http_function_;
};

class HttpAsyncServer : public TcpAsyncServerBase {
public:
  HttpAsyncServer(boost::asio::io_context& io, const String& host, u32 port, u64 ms_timeout = 0);
  auto SetTimeOut(u64 ms_timeout) -> void;

protected:
  using tcp = boost::asio::ip::tcp;
  void HandleAccept(std::shared_ptr<tcp::socket> socket, boost::system::error_code ec) override;

private:
  u64 ms_timeout_ = 0;
  VecDeque<std::shared_ptr<HttpAsyncClient>> clients_;
};

#endif  // HTTPSERVER_H
