#include "TcpClient.h"
#include <boost/asio.hpp>
namespace asio = boost::asio;

void TcpSession::SetConnectionHandler(std::function<void(const TcpInformation&)> conn_handler) {
  conn_handler_ = std::move(conn_handler);
}

void TcpSession::SetMsgHandler(std::function<Vec<char>(const Vec<char>&)> msg_handler) {
  msg_handler_ = std::move(msg_handler);
}
TcpSession::TcpSession() : io_(), read_buf_(1024) {
  sockfd_ = boost::asio::ip::tcp::socket(io_);
}

void TcpSession::Close() {
  // sockfd_->close();
  // io_.stop();
}
void TcpSession::Create(String host, int port) {
  host_ = std::move(host);
  port_ = port;
}

void TcpSession::StartRecv() {
  sockfd_->async_read_some(read_buf_.AsioBuffer(),
                           [this](boost::system::error_code ec, size_t bytes_len) {
                             if (ec.failed()) {
                               fmt::println("connection failed");
                               return;
                             }
                             return HandleRead(ec, bytes_len);
                           });
}
void TcpSession::HandleConn(boost::system::error_code ec) {
  if (conn_handler_) {
    TcpInformation info;
    info.host = host_;
    info.port = port_;
    info.sockfd = sockfd_->native_handle();
    conn_handler_(info);
  }
}

void TcpSession::Send(Vec<char> msg) {
  sockfd_->async_write_some(asio::buffer(msg.data(), msg.size()),
                            [this, msg](boost::system::error_code ec, u64 bytes_transferred) {
                              fmt::println("{}", mnet::VecBuf2String(msg));
                              if (ec.failed()) {
                                fmt::println("failed");
                              }
                              StartRecv();
                            });
}
void TcpSession::Send(const String& msg) {
  return Send(mnet::MakeVecBuf(msg));
}
void TcpSession::Wait() {
  io_thread_ = std::thread([this]() {
    io_.run();
  });
  io_thread_.join();
}
void TcpSession::SetAfterMsgHandler(std::function<void()> handler) {
  after_msg_handler_ = std::move(handler);
}
void TcpSession::HandleRead(boost::system::error_code ec, size_t bytes_len) {
  if (msg_handler_) {
    auto send_back_msg = msg_handler_(read_buf_.PopFront(std::numeric_limits<u64>::max()));
    if (!send_back_msg.empty()) {
      sockfd_->async_write_some(asio::buffer(send_back_msg.data(), send_back_msg.size()),
                                [this](boost::system::error_code ec, u64 bytes_transferred) {
                                  if (ec.failed()) {
                                    fmt::println("write falied");
                                  }
                                  if (after_msg_handler_) {
                                    after_msg_handler_();
                                  }
                                });
    }
  }
  StartRecv();
}
void TcpSession::Start() {
  sockfd_->async_connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(host_), port_),
                         [this](boost::system::error_code ec) {
                           HandleConn(ec);
                         });
}