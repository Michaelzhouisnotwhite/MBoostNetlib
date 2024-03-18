#include "TcpServer.h"

#include <utility>
namespace asio = boost::asio;

TcpAsyncServerBase::TcpAsyncServerBase(asio::io_context& ioc, const String& host, u32 port)
    : ioc_(ioc),
      accepter_(asio::make_strand(ioc),
                tcp::endpoint(asio::ip::address_v4::from_string(host), port)) {
}

void TcpAsyncServerBase::Start() {
  StartAccept();
}

void TcpAsyncServerBase::StartAccept() {
  auto client = std::make_shared<tcp::socket>(ioc_);
  accepter_.async_accept(*client, [this, client](boost::system::error_code ec) {
    HandleAccept(client, ec);
    StartAccept();
  });
}
TcpAsyncClient::TcpAsyncClient(std::shared_ptr<tcp::socket> client_sock)
    : socket_(std::move(client_sock)), recv_buffer_(1024) {
}
void TcpAsyncClient::StartRecv() {
  socket_->async_read_some(recv_buffer_.AsioBuffer(),
                           [self = shared_from_this()](const boost::system::error_code& error,
                                                       std::size_t bytes_transferred) {
                             self->HandleReadSome(error, bytes_transferred);
                           });
}
void TcpAsyncClient::SetHandleFunc(std::function<Vec<char>(Vec<char>)> callback) {
  call_back_ = std::move(callback);
}
void TcpAsyncClient::HandleReadSome(const boost::system::error_code& error,
                                    std::size_t bytes_transferred) {
  if (error.failed()) {
    mhlPrinter.Println("conn closed");
    return;
  }
  auto _front = recv_buffer_.Front(std::numeric_limits<u64>::max());
  if (!call_back_) {
    return;
  }
  auto res = call_back_(_front);

  // mhlPrinter.Print(fmt::format("HandleReadSome error: {}\n", error.message()),
  //                  mnet::ThreadPrinter::Color_Red);
  recv_buffer_.PopFront(bytes_transferred < _front.size() ? _front.size() : bytes_transferred);
  socket_->async_write_some(
      asio::buffer(res.data(), res.size()),
      [self = shared_from_this()](const boost::system::error_code& _ec, std::size_t) {
        if (_ec.failed()) {
          mhlPrinter.Println(fmt::format("resp error: {}", _ec.message()),
                             mnet::ThreadPrinter::Color_Red);
        }
      });

  StartRecv();
}

TcpAsyncServer::TcpAsyncServer(boost::asio::io_context& ioc, const String& host, u32 port)
    : TcpAsyncServerBase(ioc, host, port) {
}
void TcpAsyncServer::SetHandler(decltype(TcpAsyncClient::call_back_) callback) {
  call_back_ = std::move(callback);
}

void TcpAsyncServer::HandleAccept(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                                  boost::system::error_code ec) {
  if (ec.failed()) {
    fmt::println("{}", ec.message());
    return;
  }
  mhlPrinter.Println(fmt::format("success peer addr: {}:{}",
                                 socket->remote_endpoint().address().to_string(),
                                 socket->remote_endpoint().port()));
  auto _client = std::make_shared<TcpAsyncClient>(socket);
  _client->SetHandleFunc(call_back_);
  _client->StartRecv();
}