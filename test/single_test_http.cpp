#include <TcpServer.h>
#include <toy/type.h>
#include <boost/asio.hpp>
#include <cstdio>
#include "HttpServer.h"
#include "fmt/core.h"
int main(int argc, char* argv[]) {
    boost::asio::io_context io;
    auto server = HttpAsyncServer(io, "0.0.0.0", 8888);
    server.Start();
    io.run();
    return 0;
}
