#include <iostream>
#include <thread>
#include <asio.hpp>
#include <asio/ssl.hpp>
#include <string>

using asio::ip::tcp;

void send(std::shared_ptr<asio::ssl::stream<asio::ip::tcp::socket>> soc, int count)
{
    asio::async_write(*soc, asio::buffer("LOGIN user" + std::to_string(count) + " pass1234\n"),
        [soc, count](std::error_code ec, std::size_t) {
            if (ec) {
                std::cout << "쓰기 실패: " << ec.message() << "\n";
            }
            else {
                std::cout << "쓰기 성공\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));

            send(soc, count);
        });
}

int main() {
    std::vector<std::thread> testpool;
    for (int count = 1; count < 500; ++count)
    {
        testpool.emplace_back([count]() {
            asio::io_context io_context;
            asio::ssl::context ssl_ctx(asio::ssl::context::tlsv12_client);
            ssl_ctx.set_verify_mode(asio::ssl::verify_none); // 테스트용

            auto ssl_stream = std::make_shared<asio::ssl::stream<tcp::socket>>(io_context, ssl_ctx);

            tcp::resolver resolver(io_context);
            auto endpoints = resolver.resolve("127.0.0.1", "9000");

            asio::connect(ssl_stream->lowest_layer(), endpoints);
            std::cout << std::to_string(count) << " : TCP 연결완료" << std::endl;
            ssl_stream->async_handshake(asio::ssl::stream_base::client,
                [ssl_stream, count](const std::error_code& ec) {
                    if (!ec) {
                        std::cout << "핸드셰이크 완료" << std::endl;
                        send(ssl_stream,count);
                    }
                    else {
                        std::cout << "[클라 " << count << "] handshake 에러: " << ec.message() << "\n";
                    }
                });

            io_context.run();
            });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    for (auto& t : testpool) {
        t.join();
    }
        /*while (true) {
            std::string msg;
            std::getline(std::cin, msg);
            msg += "\n";
        }*/


    return 0;
}
