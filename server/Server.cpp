#include "pch.h"
#include "Server.hpp"
#include "Session.hpp"

Server::Server(asio::io_context& io_context, short port)
    : acceptor(io_context, asio::ip::tcp::endpoint(tcp::v4(), port))
    , io_context(io_context)
    , ssl_ctx(asio::ssl::context::tlsv12_server)    // TLS 버젼
{
    try
    {
        // std::cout << "현재 경로: " << std::filesystem::current_path() << "\n";

        // TLS 키 세팅
        ssl_ctx.use_certificate_chain_file("server.crt");
        ssl_ctx.use_private_key_file("server.key", asio::ssl::context::pem);
    }
    catch (const std::exception& ex)
    {
        std::cout << "키를 찾을 수 없습니다." << ex.what() << std::endl;
    }
    std::cout << "[Server::Server()] : 연결 대기중" << std::endl;
    start_accept();
}


void Server::start_accept()
{
    auto socket = std::make_shared<tcp::socket>(io_context);
    acceptor.async_accept(*socket, [this, socket](std::error_code ec) mutable {
        if (!ec) {
            auto ssl_stream = std::make_shared<asio::ssl::stream<tcp::socket>>(std::move(*socket), ssl_ctx);
            ssl_stream->async_handshake(asio::ssl::stream_base::server,
                [this, ssl_stream](std::error_code ec) {
                    if (!ec) {
                        auto session = std::make_shared<Session>(ssl_stream);
                        session->start();
                    }
                    else {
                        std::cout << "[Server::async_accept] : 에러 -> " << ec.message() << std::endl;
                    }
                });
            std::cout << "[Server::async_accept] : 클라이언트 연결 완료, 수신을 시작합니다." << std::endl;
        }
        else {

            std::cout << "[Server::async_accept] : 에러 -> " << ec.message() << std::endl;
        }

        start_accept();
        });
}
