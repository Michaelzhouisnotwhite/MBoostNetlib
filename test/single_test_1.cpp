#include <TcpServer.h>
#include <toy/type.h>
#include <boost/asio.hpp>
#include <cstdio>
#include "fmt/core.h"
class MyTcpServer : public TcpAsyncServerBase {
public:
    MyTcpServer(boost::asio::io_context& io) : TcpAsyncServerBase(io, "0.0.0.0", 8888) {
    }

protected:
    void HandleAccept(std::shared_ptr<boost::asio::ip::tcp::socket> socket,
                      boost::system::error_code ec) override {
        if (ec.failed()) {
            fmt::println("{}", ec.message());
            return;
        }
        fmt::println("success peer addr: {}:{}", socket->remote_endpoint().address().to_string(),
                     socket->remote_endpoint().port());
    }
};

int main(int argc, char* argv[])
{
    boost::asio::io_context io;
    auto server = MyTcpServer(io);
    server.Start();
    io.run();
    return 0;
}
