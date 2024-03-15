#include "TcpClient.h"

TcpConnection::TcpConnection(boost::asio::io_context& io, const String& host, const int port)
    : host_(host), port_(port), sockfd_(io) {
}

TcpSession::TcpSession() : io_() {
}