#include "HttpServer.h"
#include <iostream>

#include "fmt/color.h"
#include "fmt/core.h"
namespace asio = boost::asio;
using namespace mhttplib;
namespace http = boost::beast::http;

HttpAsyncClient::HttpAsyncClient(std::shared_ptr<tcp::socket> socket)
    : socket_(std::move(socket)), recv_buffer_(buf_size_, '\0') {
  on_http_function_ = [](std::shared_ptr<HttpRequest> req) -> std::shared_ptr<HttpBaseResponse> {
    auto resp = std::make_shared<HttpBaseResponse>();
    resp->StatusCode(200);
    resp->header.SetContentType("application/json");
    resp->body = R"({"message": "from mHttpServer"})";
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
    print(fg(fmt::color::red), "HandleReadSome error: {}\n", error.message());
    return;
  }
  fmt::print(fg(fmt::color::red), "HandleReadSome error: {}\n", error.message());
  assert(bytes_transferred <= recv_buffer_.ByteSize() && "bytes_transferred < buf.bytesize()");
  // recv_buf_ = VecDeque<char>(buf.begin(), buf.begin() + bytes_transferred);
  boost::system::error_code ec;
  fmt::println("{}", recv_buffer_.PrettyStr());
  auto _front = recv_buffer_.Front(std::numeric_limits<u64>::max());

  auto bytes_putted = req_.put(asio::buffer(_front.data(), _front.size()), ec);
  if (ec.failed() && ec != boost::beast::http::error::need_more) {
    fmt::print(fg(fmt::color::red), "HandleReadSome req error: {}\n", ec.message());
    return;
  }
  if (ec == boost::beast::http::error::need_more && bytes_putted == 0 && !_front.empty()) {
    recv_buffer_.Resize(recv_buffer_.size() * 2);
    StartRecv();
    return;
  }
  recv_buffer_.PopFront(bytes_putted);

  if (req_.is_header_done()) {
    // print header
    fmt::println("header: ------------");
    for (const auto& map : req_.get()) {
      fmt::println("{}: {}", std::string(map.name_string()), std::string(map.value()));
    }
    if (req_.content_length().has_value() && !req_.is_done()) {
      // TODO: do something
      _front = recv_buffer_.Front(std::numeric_limits<u64>::max());

      bytes_putted = req_.put(asio::buffer(_front.data(), _front.size()), ec);
      if (bytes_putted != _front.size()) {
        return;
      }
      recv_buffer_.PopFront(bytes_putted);
    }
  }
  if (req_.is_done()) {
    fmt::println("body: -----------\n{}", req_.get().body());
    // do other things
    if (on_http_function_) {
      auto req = std::make_shared<mhttplib::HttpRequest>();
      if (!req) {
        return;
      }
      for (const auto& ele : req_.get()) {
        req->header[ele.name_string()] = ele.value();
      }
      req->body = req_.get().body();
      auto resp = on_http_function_(req);
      resp->Prepare();
      auto resp_chunk = resp->Read(100);
      while (!resp_chunk.empty()) {
        socket_->async_write_some(
            asio::buffer(resp_chunk.data(), resp_chunk.size()),
            [self = shared_from_this()](boost::system::error_code _ec, std::size_t) {
              fmt::print(fg(fmt::color::red), "resp error: {}\n", _ec.message());
            });
        resp_chunk = resp->Read(100);
      }
      socket_->shutdown(tcp::socket::shutdown_send, ec);
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

  fmt::println("success peer addr: {}:{}", socket->remote_endpoint().address().to_string(),
               socket->remote_endpoint().port());

  auto _client = std::make_shared<HttpAsyncClient>(socket);
  _client->StartRecv();
}
