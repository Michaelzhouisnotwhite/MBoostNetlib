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
  sockfd_ = boost::asio::ip::tcp::socket(asio::make_strand(io_));
}

void TcpSession::Close() {
  sockfd_->close();
  io_.stop();
}
void TcpSession::Create(String host, int port) {
  host_ = std::move(host);
  port_ = port;
}

void TcpSession::StartRecv() {
  sockfd_->async_read_some(read_buf_.AsioBuffer(),
                           [this](boost::system::error_code ec, size_t bytes_len) {
                             HandleRead(ec, bytes_len);
                           });
}

void TcpSession::StartSend() {
  if (send_buf_.empty()) {
    StartRecv();
  } else {
    const auto& buf = send_buf_.front().first;
    auto cb = send_buf_.front().second;
    sockfd_->async_write_some(asio::buffer(buf.data(), buf.size()),
                              [this, buf, cb](boost::system::error_code ec, u64 bytes_transferred) {
                                fmt::println("{}", mnet::VecBuf2String(buf));
                                if (ec.failed()) {
                                  fmt::println("failed");
                                } else {
                                  send_buf_.pop_front();
                                  if (cb) {
                                    cb();
                                  }
                                  StartSend();
                                }
                              });
  }
}
void TcpSession::HandleConn(boost::system::error_code ec) {
  if (ec.failed()) {
    fmt::println("conn failed");
    return;
  }
  if (conn_handler_) {
    TcpInformation info;
    info.host = host_;
    info.port = port_;
    info.sockfd = sockfd_->native_handle();
    conn_handler_(info);
  }
  StartSend();
}

void TcpSession::Send(Vec<char> msg, std::function<void()> cb) {
  send_buf_.emplace_back(msg, cb);
}
void TcpSession::Send(const String& msg, std::function<void()> cb) {
  return Send(mnet::MakeVecBuf(msg), std::move(cb));
}
void TcpSession::Wait() {
  io_thread_.join();
}
void TcpSession::SetAfterMsgHandler(std::function<void()> handler) {
  after_msg_handler_ = std::move(handler);
}
void TcpSession::HandleRead(boost::system::error_code ec, size_t bytes_len) {
  if (ec.failed()) {
    fmt::println("read failed");
    return;
  }
  if (msg_handler_) {
    auto send_back_msg = msg_handler_(read_buf_.PopFront(std::numeric_limits<u64>::max()));
    if (!send_back_msg.empty()) {
      this->Send(send_back_msg, after_msg_handler_);
    }
  }
  StartSend();
}
void TcpSession::Start() {
  io_thread_ = std::thread([this]() {
    io_.run();
  });
  sockfd_->async_connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(host_), port_),
                         [this](boost::system::error_code ec) {
                           HandleConn(ec);
                         });
}