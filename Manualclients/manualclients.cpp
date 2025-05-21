#include <iostream>
#include <thread>
#include <asio.hpp>
#include <asio/ssl.hpp>

using asio::ip::tcp;

int main() {
    try {
        asio::io_context io_context;

        asio::ssl::context ssl_ctx(asio::ssl::context::tlsv12_client);
        ssl_ctx.set_verify_mode(asio::ssl::verify_none); // 테스트용

        asio::ssl::stream<tcp::socket> ssl_stream(io_context, ssl_ctx);

        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "9000");
        asio::connect(ssl_stream.next_layer(), endpoints);

        ssl_stream.async_handshake(asio::ssl::stream_base::client,
            [&](const std::error_code& ec) {
                if (!ec) {
                    std::cout << "[클라] SSL 핸드셰이크 성공\n";

                    // 수신 스레드
                    std::thread reader([&]() {
                        while (true) {
                            asio::streambuf buf;
                            asio::read_until(ssl_stream, buf, "\n");

                            std::istream is(&buf);
                            std::string line;
                            std::getline(is, line);
                            std::cout << "[서버]: " << line << "\n";
                        }
                        });
                    reader.detach();
                }
                else {
                    std::cerr << "[클라] 핸드셰이크 실패: " << ec.message() << "\n";
                }
            });

        // io_context를 돌려야 위 콜백도 실행됨
        std::thread io_thread([&]() {
            io_context.run();
            });

        // 메인 스레드에서 송신 루프
        while (true) {
            std::string msg;
            std::getline(std::cin, msg);
            msg += "\n";
            
            asio::write(ssl_stream, asio::buffer(msg));
        }

        io_thread.join();
    }
    catch (const std::exception& e) {
        std::cerr << "에러: " << e.what() << "\n";
    }

    return 0;
}
