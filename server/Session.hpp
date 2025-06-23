#pragma once
#include "Player.hpp"
#include "myStruct.hpp"
#include "utils.hpp"
using asio::ip::tcp;

enum class ReadState
{
	ReadingHeader,
	ReadingBody
};

class Session : public std::enable_shared_from_this<Session> {
private:
	int id;
	bool isAuth = false;

	ReadState state = ReadState::ReadingHeader;
	std::vector<char> read_buffer;
	int expected_length = 0;

	std::shared_ptr<asio::ssl::stream<tcp::socket>> ssl_stream;
	std::mutex writeMutex;

	std::string read_msg;
	std::queue<std::shared_ptr<std::string>> writeQueue;
	bool sending;

	Player player;

public:
	/////////////////////////////
	// 생성자
	// TLS 사용시
	Session(std::shared_ptr<asio::ssl::stream<tcp::socket>> stream);
	// 아래생성자 사용금지, 보안 위험
	// Session(std::shared_ptr<tcp::socket> _socket); 
	/////////////////////////////

	void start();
	void push_WriteQueue(std::shared_ptr<std::string> msg);
	void Close();


	/////////////////////////////
	// get, set
	std::shared_ptr<asio::ssl::stream<tcp::socket>> get_stream() const { return ssl_stream; }
	Player& get_player() { return player; }
	int get_id() const { return id; }
	/////////////////////////////

private:
	void do_read();
	void do_write();
	bool isValid(const std::string& packet);

	bool validateLogin(const uint8_t* data, size_t size);
	bool validateMove(const uint8_t* data, size_t size);
	bool validateAttack(const uint8_t* data, size_t size);
};