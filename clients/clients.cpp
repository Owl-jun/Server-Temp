#include <iostream>
#include <asio.hpp>

using asio::ip::tcp;

int main()
{
    try
    {
        asio::io_context io_context;

        // 서버 IP와 포트 (원하는대로 수정)
        std::string server_ip = "127.0.0.1";
        int server_port = 9000;

        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(server_ip, std::to_string(server_port));

        tcp::socket socket(io_context);
        asio::connect(socket, endpoints);

        std::cout << "서버 연결됨.\n";

        std::thread([&]() {
            while (true) {
                asio::streambuf buf;
                asio::read_until(socket, buf, "\n");

                std::istream is(&buf);
                std::string received_msg;
                std::getline(is, received_msg);

                std::cout << "수신: " << received_msg << "\n";
            }
            }).detach();
        
        while (true)
        {
            std::string msg;
            std::cout << "> ";
            std::getline(std::cin, msg);

            msg = msg + "\n";
            asio::write(socket, asio::buffer(msg));
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "에러: " << e.what() << "\n";
    }

    return 0;
}
