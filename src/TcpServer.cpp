#include "TcpServer.h"
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
