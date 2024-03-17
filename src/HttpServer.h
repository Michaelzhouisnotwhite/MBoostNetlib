#ifndef SRC_HTTPSERVER
#define SRC_HTTPSERVER
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
  ~HttpAsyncClient();

private:
  void SetHandleFunc(std::function<std::shared_ptr<mnet::HttpResponseBase>(
                         std::shared_ptr<mnet::HttpRequest>)> cb);
  void StartRecv();
  void HandleReadSome(const boost::system::error_code& error, std::size_t bytes_transferred);
  u64 buf_size_ = 8000;
  std::shared_ptr<tcp::socket> socket_;
  mnet::VecBuffer<char> recv_buffer_;
  mnet::HttpRequestParser req_;
  std::function<std::shared_ptr<mnet::HttpResponseBase>(std::shared_ptr<mnet::HttpRequest>)>
      on_http_function_;
};

using HttpHandlerReq= std::shared_ptr<mnet::HttpRequest>;
using HttpHandlerResp= std::shared_ptr<mnet::HttpResponseBase>;
class HttpAsyncServer : public TcpAsyncServerBase {
public:
  HttpAsyncServer(boost::asio::io_context& io, const String& host, u32 port, u64 ms_timeout = 0);
  auto SetTimeOut(u64 ms_timeout) -> void;
  auto SetHttpHandler(decltype(HttpAsyncClient::on_http_function_) cb) -> void;

protected:
  using tcp = boost::asio::ip::tcp;
  void HandleAccept(std::shared_ptr<tcp::socket> socket, boost::system::error_code ec) override;

private:
  decltype(HttpAsyncClient::on_http_function_) http_handler_;
  u64 ms_timeout_ = 0;
  VecDeque<std::shared_ptr<HttpAsyncClient>> clients_;
};

#endif /* SRC_HTTPSERVER */
