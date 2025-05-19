#pragma once
class Session;

class Server {
private:
	asio::ip::tcp::acceptor acceptor;
	asio::io_context& io_context;
	asio::ssl::context ssl_ctx;
public:
	Server(asio::io_context& io_context, short port);
private:
	void start_accept();
};