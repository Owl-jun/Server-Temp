#include "pch.h"
#include "Session.h"

class Server {
private:
	asio::ip::tcp::acceptor acceptor;
	asio::io_context& io_context;
	asio::ssl::context ssl_ctx;
public:
	Server(asio::io_context& io_context, short port)
		: acceptor(io_context, asio::ip::tcp::endpoint(tcp::v4(), port))
		, io_context(io_context)
        , ssl_ctx(asio::ssl::context::tlsv12_server)
	{
		try
		{
            //std::cout << "현재 경로: " << std::filesystem::current_path() << "\n";

            if (!std::filesystem::exists("server.crt"))
                std::cerr << "❌ server.crt 파일 없음\n";

            if (!std::filesystem::exists("server.key"))
                std::cerr << "❌ server.key 파일 없음\n";

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

private:
	void start_accept()
	{
        auto socket = std::make_shared<tcp::socket>(io_context);
        std::cout << "[Server::start_accept] : 클라이언트 연결 수신 시작" << std::endl;

        acceptor.async_accept(*socket, [this, socket](std::error_code ec) mutable {
            auto ssl_stream = std::make_shared<asio::ssl::stream<tcp::socket>>(std::move(*socket), ssl_ctx);
            if (!ec) {
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
                std::cout << "[Server::async_accept] : 클라이언트 연결 완료" << std::endl;
            }
            else {

                std::cout << "[Server::async_accept] : 에러 -> " << ec.message() << std::endl;
            }

            start_accept();  
        });
	}
};