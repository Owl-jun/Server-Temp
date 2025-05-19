#include "pch.h"
#include "Session.hpp"
#include "QueueManager.hpp"
#include "SessionManager.hpp"
#include "myStruct.hpp"
//#include "utils.hpp"


Session::Session(std::shared_ptr<asio::ssl::stream<tcp::socket>> stream)
	: ssl_stream(stream)
	, sending(false)
	, player()
	, id(SessionManager::make_UniqueId())
{
	spdlog::info(std::format("[Session] 세션 생성 세션ID : {}", id));
}

void Session::start()
{
	SessionManager::GetInstance().AddSession(shared_from_this());
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
			if (self->isValid(msg)) { QueueManager::GetInstance().push({ self,msg }); }
			
			self->do_read();
		}
		else
		{
			spdlog::error("[Session::do_read] : 예외 발생 -> " + ec.message());
			self->Close();
		}
		});
}

void Session::push_WriteQueue(std::shared_ptr<std::string> msg)
{
	std::lock_guard<std::mutex> lock(writeMutex);
	writeQueue.push(msg);

	// [250518] issue : Lock 중첩 데드락 발생 , post 로 수정
	// self->do_write();
	asio::post(ssl_stream->get_executor(), [self = shared_from_this()]() {
		self->do_write();
	});
	
}

void Session::do_write()
{
	std::lock_guard<std::mutex> lock(writeMutex);

	if (sending || writeQueue.empty())
		return;

	sending = true;
	auto msg = writeQueue.front();
	auto self = shared_from_this();

	asio::async_write(*ssl_stream, asio::buffer(*msg),
		[self](std::error_code ec, std::size_t)
		{
			std::lock_guard<std::mutex> lock(self->writeMutex);
			if (!ec)
			{
				self->writeQueue.pop();
				self->sending = false;

				if (!self->writeQueue.empty())
				{
					// [250518] issue : Lock 중첩 데드락 발생 , post 로 수정
					// self->do_write();
					asio::post(self->ssl_stream->get_executor(), [self]() {
						self->do_write();
					});
				}
			}
			else
			{
				spdlog::info("[Session::do_write()] 에러 발생 -> {} ", ec.message());
				self->Close();
			}
		});
}

bool Session::isValid(const std::string& packet)
{
	std::istringstream iss(packet);
	std::string id;
	iss >> id;

	std::vector<std::string>& validIDs = SessionManager::GetInstance().getValidIds();
	
	if (std::find(validIDs.begin(), validIDs.end(), id) != validIDs.end()) {
		spdlog::info(std::format("[Session::isValid] 유효성 검사 ID : {} -> true ",id));
		return true;
	}
	else {
		spdlog::error(std::format("[Session::isValid] 유효성 검사 ID : {} -> false ", id));
		return false;
	}
}

void Session::Close()
{
	std::cout << "[Session::Close] 세션 종료" << std::endl;
	spdlog::info("[Session::Close] 세션 ID : {}", std::to_string(id));
	std::error_code ec;
	ssl_stream->shutdown(ec);
	SessionManager::GetInstance().DelSession(id);
}

