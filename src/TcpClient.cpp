#include "TcpClient.h"
#include <boost/asio.hpp>
namespace asio = boost::asio;

void TcpSession::SetConnectionHandler(std::function<void(const TcpInformation&)> conn_handler) {
  conn_handler_ = std::move(conn_handler);
}

void TcpSession::SetMsgHandler(std::function<Vec<char>(const Vec<char>&)> msg_handler) {
  msg_handler_ = std::move(msg_handler);
}
TcpSession::TcpSession(boost::asio::io_context& io) : io_(io), sockfd_(io), read_buf_(1024) {
}

void TcpSession::Create(String host, int port) {
  host_ = std::move(host);
  port_ = port;
}
void TcpSession::HandleConn(boost::system::error_code ec) {
  if (conn_handler_) {
    TcpInformation info;
    info.host = host_;
    info.port = port_;
    info.sockfd = sockfd_.native_handle();
    conn_handler_(info);
  }
  sockfd_.async_read_some(read_buf_.AsioBuffer(),
                          [this](boost::system::error_code ec, size_t bytes_len) {
                            return HandleRead(ec, bytes_len);
                          });
}
void TcpSession::HandleRead(boost::system::error_code ec, size_t bytes_len) {
  if (msg_handler_) {
    auto send_back_msg = msg_handler_(read_buf_.PopFront(std::numeric_limits<u64>::max()));
    sockfd_.async_write_some(asio::buffer(send_back_msg.data(), send_back_msg.size()),
                             [](boost::system::error_code ec) {

                             });
  }
}
void TcpSession::Start() {
  sockfd_.async_connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(host_), port_),
                        [this](boost::system::error_code ec) {
                          HandleConn(ec);
                        });
}