#include "pch.h"
#include "Session.h"
#include "QueueManager.h"
#include "SessionManager.hpp"

Session::Session(std::shared_ptr<asio::ssl::stream<tcp::socket>> stream)
	: ssl_stream(stream)
	, sending(false)
	, player()
	, id(SessionManager::getUniqueId())
{
}

void Session::start()
{
	SessionManager::GetInstance().AddSession(shared_from_this());
	std::cout << "[Session::start()] : 세션 시작됨" << std::endl;
	do_read();
}


void Session::do_read()
{
	auto self(shared_from_this());
	asio::async_read_until(*ssl_stream, asio::dynamic_buffer(read_msg), "\n", [self](std::error_code ec, std::size_t length) {
		if (!ec)
		{
			std::string msg = self->read_msg.substr(0, length - 1);
			self->read_msg.erase(0, length);

			if (self->isValid(msg)) {
				QueueManager::GetInstance().push({ self,msg });
				spdlog::info("[Session::do_read] : 클라이언트 -> " + msg + "Task Queue PUSH 완료.");
			}
			else {
				spdlog::error("[Session::do_read] : 클라이언트 -> " + msg + "Task 폐기");
			}

			self->do_read();
		}
		else
		{
			std::cout << "[Session::do_read] : 예외 발생 -> " << ec.message() << std::endl;
			spdlog::error("[Session::do_read] : 예외 발생 -> " + ec.message());

			self->Close();
		}
		});
}

void Session::push_WriteQueue(std::shared_ptr<std::string> msg)
{
	std::unique_lock<std::mutex> lock(writeMutex);
	writeQueue.push(msg);
	//std::cout << "[Session::push_WriteQueue] : lock 획득 -> " << *msg << " writeQueue 등록완료." << std::endl;
	lock.unlock();

	if (!sending && !writeQueue.empty())
	{
		sending = true;
		do_write();
	}
}

void Session::set_chat_id(std::string id)
{
	player.nickname = id;
}

void Session::set_pos(double _x, double _y)
{
	player.position = { _x,_y };
}

std::string Session::get_position()
{
	return player.get_PlayerPos();
}



std::string Session::get_chat_id()
{
	return player.nickname;
}

void Session::do_write()
{
	std::lock_guard<std::mutex> lock(writeMutex);

	if (writeQueue.empty()) {
		sending = false;
		std::cout << "[Session::do_write] writeQueue 가 비었음 " << std::endl;
		return;
	}

	std::shared_ptr<std::string> msg = writeQueue.front();
	std::cout << "[Session::do_write] 송신 시작 -> " << *msg << std::endl;

	auto self(shared_from_this());
	asio::async_write(*ssl_stream, asio::buffer(*msg),
		[self](std::error_code ec, std::size_t)
		{
			std::lock_guard<std::mutex> lock(self->writeMutex);

			if (!ec)
			{
				self->writeQueue.pop();

				if (!self->writeQueue.empty())
				{
					/// [250517] issue : Mutex 재귀 호출로 인한 데드락 발생 -> post 사용, do_write 종료 후 실행으로 해결 (250518)
					asio::post(self->ssl_stream->get_executor(), [self]() {
						self->do_write();
					});
				}
				else 
				{
					self->sending = false;
				}
			}
			else
			{
				std::cout << "[Session::do_write()] 에러 발생 -> " << ec.message() << std::endl;
				self->Close();
			}
		});
}

bool Session::isValid(const std::string& packet)
{
	std::istringstream iss(packet);
	std::cout << "[Session::isValid] packet -> " << packet << std::endl;
	std::string id;
	iss >> id;
	std::cout << "[Session::isValid] id -> " << id << std::endl;
	std::vector<std::string>& validIDs = SessionManager::GetInstance().getValidIds();
	if (std::find(validIDs.begin(), validIDs.end(), id) != validIDs.end()) {
		std::cout << "[Session::isValid] 유효성 검사 -> true " << std::endl;
		return true;
	}
	else {
		std::cout << "[Session::isValid] 유효성 검사 -> false " << std::endl;
		return false;
	}
}

void Session::Close()
{
	std::cout << "[Session:Close] 세션 종료됨" << std::endl;
	std::error_code ec;
	ssl_stream->shutdown(ec);

	SessionManager::GetInstance().DelSession(id);
}
