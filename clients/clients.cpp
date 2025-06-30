#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <asio.hpp>
#include <asio/ssl.hpp>

using asio::ip::tcp;

void create_tls_dummy(int id, const std::string& host, int port, asio::ssl::context& ctx) {
    try {
        asio::io_context io;
        tcp::resolver resolver(io);
        asio::ssl::stream<tcp::socket> socket(io, ctx);

        auto endpoints = resolver.resolve(host, std::to_string(port));
        asio::connect(socket.next_layer(), endpoints);
        socket.handshake(asio::ssl::stream_base::client);

        //std::cout << "[" << id << "] TLS 연결 성공\n";

        while (true) {
            // 1. 현재 시간(ms)
            auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();

            std::string payload = std::to_string(now);
            uint8_t opcode = 0x09;

            uint32_t len = htonl(payload.size() + 1);
            std::vector<uint8_t> full_packet(sizeof(len) + payload.size() + 1);
            memcpy(full_packet.data(), &len, sizeof(len));
            memcpy(full_packet.data() + sizeof(len), &opcode, 1);
            memcpy(full_packet.data() + sizeof(len) + 1, payload.data(), payload.size());

            // send
            try {
                auto bytes_sent = asio::write(socket, asio::buffer(full_packet));
                //std::cout << "[" << id << "] 보낸 바이트 수: " << bytes_sent << " byte\n";
            }
            catch (std::exception& e) {
                std::cerr << "[" << id << "] 전송 중 예외 발생: " << e.what() << "\n";
            }

            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

    }
    catch (std::exception& e) {
        std::cerr << "[" << id << "] 예외 발생: " << e.what() << "\n";
    }
}



int main() {
    int num;
    std::cout << "더미 TLS 클라이언트 수: ";
    std::cin >> num;

    asio::ssl::context ctx(asio::ssl::context::sslv23);
    ctx.set_verify_mode(asio::ssl::verify_none); // 테스트용: 인증서 검증 생략

    std::vector<std::thread> threads;
    for (int i = 0; i < num; ++i) {
        threads.emplace_back([i, &ctx]() {
            create_tls_dummy(i, "127.0.0.1", 9000, ctx);
            });
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 서버 부하 방지
    }

    std::cout << num << "개 TLS 연결 완료.\n";
    std::cout << "종료하려면 q 입력\n";
    while (true) {
        char c;
        std::cin >> c;
        if (c == 'q') break;
    }

    return 0;
}
