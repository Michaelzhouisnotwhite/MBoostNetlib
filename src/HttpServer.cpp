#include "HttpServer.h"
#include <iostream>

#include "fmt/color.h"
#include "fmt/core.h"
namespace asio = boost::asio;
using namespace mhttplib;

HttpAsyncClient::HttpAsyncClient(std::shared_ptr<tcp::socket> socket)
    : socket_(std::move(socket)), recv_buffer_(buf_size_, '\0') {
  on_http_function_ =
      [](const std::shared_ptr<HttpRequest>& req) -> std::shared_ptr<HttpBaseResponse> {
    auto resp = std::make_shared<HttpBaseResponse>();
    resp->StatusCode(200);
    resp->header.SetContentType("application/json");
    resp->body =
        fmt::format(R"({{"message": "from mHttpServer", "origin_content": {}}})", req->body);
    return resp;
  };
}

void HttpAsyncClient::StartRecv() {
  socket_->async_read_some(recv_buffer_.AsioBuffer(),
                           [self = shared_from_this()](const boost::system::error_code& error,
                                                       std::size_t bytes_transferred) {
                             self->HandleReadSome(error, bytes_transferred);
                           });
}
void HttpAsyncClient::HandleReadSome(const boost::system::error_code& error,
                                     std::size_t bytes_transferred) {
  if (error.failed()) {
    mhlPrinter.Print(fmt::format("HandleReadSome error: {}\n", error.message()),
                     ThreadPrinter::Color_Red);
    return;
  }
  mhlPrinter.Print(fmt::format("HandleReadSome error: {}\n", error.message()),
                   ThreadPrinter::Color_Red);
  assert(bytes_transferred <= recv_buffer_.ByteSize() && "bytes_transferred <= buf.bytesize()");
  mhlPrinter.Println(recv_buffer_.PrettyStr());

  auto _front = recv_buffer_.Front(std::numeric_limits<u64>::max());

  int req_ec;
  auto bytes_putted = req_.Put(_front, req_ec);
  if (req_ec == HttpRequestParser::bad) {
    boost::system::error_code ec;
    mhlPrinter.Print(fmt::format("HandleReadSome error: {}\n", error.message()),
                     ThreadPrinter::Color_Red);
    return;
  }
  if (req_ec == HttpRequestParser::indeterminate && bytes_putted == 0 && !_front.empty()) {
    recv_buffer_.Resize(recv_buffer_.size() * 2);
    StartRecv();
    return;
  }
  recv_buffer_.PopFront(bytes_putted);

  if (req_.HeaderDone()) {
    // print header
    mhlPrinter.Println("header: ------------");
    for (const auto& [name, value] : req_.Result()->header) {
      mhlPrinter.Println(fmt::format("{}: {}", name, value));
    }
    if (auto req = req_.Result(); !req) {
      if (!req->header.chunked() && !req->header.HasContentLength()) {
        req->header["Content-Length"] = std::to_string(0);
      }
      if (!req_.Done()) {
        // TODO: do something
        _front = recv_buffer_.Front(std::numeric_limits<u64>::max());

        bytes_putted = req_.Put(_front, req_ec);
        if (bytes_putted != _front.size()) {
          return;
        }
        recv_buffer_.PopFront(bytes_putted);
      }
    }
  }
  if (req_.Done()) {
    mhlPrinter.Println(fmt::format("body: -----------\n{}", req_.Result()->body));
    // do other things
    if (on_http_function_) {
      auto req = req_.Result();
      if (!req) {
        return;
      }

      auto resp = on_http_function_(req);
      if (!resp) {
        resp = std::make_shared<HttpBaseResponse>();
      }
      resp->Prepare();
      auto resp_chunk = resp->Read(10000);
      while (!resp_chunk.empty()) {
        socket_->async_write_some(
            asio::buffer(resp_chunk.data(), resp_chunk.size()),
            [self = shared_from_this()](const boost::system::error_code& _ec, std::size_t) {
              if (_ec.failed()) {
                mhlPrinter.Println(fmt::format("resp error: {}\n", _ec.message()),
                                   ThreadPrinter::Color_Red);
              }
            });
        resp_chunk = resp->Read(10000);
      }
      req_.Reset();
      //   socket_->shutdown(tcp::socket::shutdown_send, ec);
    }
    return;
  }
  StartRecv();
}

HttpAsyncServer::HttpAsyncServer(boost::asio::io_context& io,
                                 const String& host,
                                 u32 port,
                                 u64 ms_timeout)
    : TcpAsyncServerBase(io, host, port) {
  SetTimeOut(ms_timeout);
}
auto HttpAsyncServer::SetTimeOut(u64 ms_timeout) -> void {
  ms_timeout_ = ms_timeout;
}

void HttpAsyncServer::HandleAccept(std::shared_ptr<tcp::socket> socket,
                                   boost::system::error_code ec) {
  if (ec.failed()) {
    fmt::println("{}", ec.message());
    return;
  }
  mhlPrinter.Println(fmt::format("success peer addr: {}:{}",
                                 socket->remote_endpoint().address().to_string(),
                                 socket->remote_endpoint().port()));

  auto _client = std::make_shared<HttpAsyncClient>(socket);
  _client->StartRecv();
}
