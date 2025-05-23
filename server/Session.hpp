﻿#pragma once
#include "Player.hpp"
#include "myStruct.hpp"
#include "utils.hpp"
using asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
private:
	int id;
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

};