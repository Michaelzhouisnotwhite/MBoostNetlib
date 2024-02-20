#include "HttpServer.h"
#include <iostream>
#include "fmt/core.h"
namespace asio = boost::asio;
String HttpPayload::Body() {
    return "";
}
HttpAsyncClient::HttpAsyncClient(std::shared_ptr<tcp::socket> socket) : socket_(std::move(socket)) {
}

void HttpAsyncClient::StartRecv() {
    std::array<char, 100> read_buf;
    asio::async_read(
        *socket_, asio::buffer(read_buf.data(), read_buf.size()),
        [self = shared_from_this(), &read_buf](const boost::system::error_code& error,
                                               std::size_t bytes_transferred) {
            std::cout << read_buf.data();
            self->HandleReadSome({read_buf.begin(), read_buf.end()}, error, bytes_transferred);
            read_buf.fill(0);
        });
}

void HttpAsyncClient::HandleReadSome(Vec<char> buf,
                                     const boost::system::error_code& error,
                                     std::size_t bytes_transferred) {
    if (error.failed()) {
        fmt::println("HandleReadSome error: {}", error.message());
        return;
    }
    assert(bytes_transferred <= buf.size() && "bytes_transferred < buf.size()");
    recv_buf_ = VecDeque<char>(buf.begin(), buf.begin() + bytes_transferred);
    boost::system::error_code ec;
    auto bytes_putted =
        req_.put(asio::buffer(std::vector<char>(recv_buf_.begin(), recv_buf_.end())), ec);
    if (ec.failed() && ec != boost::beast::http::error::need_more) {
        fmt::println("HandleReadSome req error: {}", ec.message());
        return;
    }

    recv_buf_.clear();
    if (req_.is_header_done()) {
        // print header
        fmt::println("header: ------------");
        for (const auto& map : req_.get()) {
            fmt::println("{}: {}", std::string(map.name_string()), std::string(map.value()));
        }
        if (req_.content_length().has_value()) {
            bytes_putted =
                req_.put(asio::buffer(std::vector<char>(recv_buf_.begin(), recv_buf_.end())), ec);
            if (!recv_buf_.empty()) {
                fmt::println("req error: {}", "recv_buf not empty");
            }
        } else {
            fmt::println("req error: {}", "no content length");
            return;
        }
    }
    if (req_.is_done()) {
        fmt::println("body: {}", req_.get().body());
        // do other things
    }
    StartRecv();
}

HttpAsyncServer::HttpAsyncServer(boost::asio::io_context& io, const String& host, u32 port)
    : TcpAsyncServerBase(io, host, port) {
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
