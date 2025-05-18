#pragma once
#include "Player.hpp"
using asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
private:
	int id;
	Player player;
	std::shared_ptr<asio::ssl::stream<tcp::socket>> ssl_stream;
	std::string read_msg;
	std::queue<std::shared_ptr<std::string>> writeQueue;
	std::mutex writeMutex;
	bool sending;
public:
	// TLS용
	Session(std::shared_ptr<asio::ssl::stream<tcp::socket>> stream);
	// Session(std::shared_ptr<tcp::socket> _socket); 평문용
	void start();
	void push_WriteQueue(std::shared_ptr<std::string> msg);
	std::shared_ptr<asio::ssl::stream<tcp::socket>> get_stream() const { return ssl_stream; }
	int get_id() const { return id; }
	void set_chat_id(std::string id);
	void set_pos(double _x, double _y);
	std::string get_position();
	std::string get_chat_id();
private:
	friend class SessionManager;

	void do_read();
	void do_write();

	bool isValid(const std::string& packet);
	void Close();

};