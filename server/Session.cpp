#include "pch.h"
#include "Session.hpp"
#include "QueueManager.hpp"
#include "SessionManager.hpp"
#include "myStruct.hpp"
#include "RedisManager.hpp"
//#include "utils.hpp"


Session::Session(std::shared_ptr<asio::ssl::stream<tcp::socket>> stream)
	: ssl_stream(stream)
	, sending(false)
	, player()
	, id(SessionManager::make_UniqueId())
{
	// spdlog::info("[Session] Created Session ID : {}, UserCount : {}", id, std::to_string(SessionManager::GetInstance().UserCount()));
}

void Session::start()
{
	SessionManager::GetInstance().AddSession(id, shared_from_this());
	std::cout << "Created Session ID , Current UserCount : " << std::to_string(SessionManager::GetInstance().UserCount()) << std::endl;
	do_read();
}

void Session::do_read()
{
	auto self(shared_from_this());

	if (state == ReadState::ReadingHeader)
	{
		// 길이 헤더: 4바이트 정수
		asio::async_read(*ssl_stream, asio::buffer(&expected_length, sizeof(int)),
			[self](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					self->expected_length = ntohl(self->expected_length); 
					if (self->expected_length > MAX_PACKET_SIZE)
					{
						spdlog::warn("비정상적으로 큰 패킷 크기 탐지 → 연결 종료");
						self->Close();
						return;
					}
					self->read_buffer.resize(self->expected_length);
					self->state = ReadState::ReadingBody;
					self->do_read();
				}
				else
				{
					spdlog::error("[Session::do_read:Header] : " + ec.message());
					self->Close();
				}
			});
	}
	else if (state == ReadState::ReadingBody)
	{
		asio::async_read(*ssl_stream, asio::buffer(read_buffer),
			[self,this](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					std::string packet(read_buffer.begin(), read_buffer.end());
					if (self->isValid(packet)) {
						QueueManager::GetInstance().push({ self, packet });
					}

					self->state = ReadState::ReadingHeader;
					self->do_read();
				}
				else
				{
					spdlog::error("[Session::do_read:Body] : " + ec.message());
					self->Close();
				}
			});
	}
}

// Delimiter 기반 수신
//void Session::do_read()
//{
//	auto self(shared_from_this());
//	asio::async_read_until(*ssl_stream, asio::dynamic_buffer(read_msg), "\n", [self](std::error_code ec, std::size_t length) {
//		if (!ec)
//		{
//			std::string msg = self->read_msg.substr(0, length - 1);
//			self->read_msg.erase(0, length);
//			if (self->isValid(msg)) { QueueManager::GetInstance().push({ self,msg }); }
//			
//			self->do_read();
//		}
//		else
//		{
//			spdlog::error("[Session::do_read] : 예외 발생 -> " + ec.message());
//			self->Close();
//		}
//		});
//}

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

// TLP 리팩토링
void Session::do_write()
{
	std::lock_guard<std::mutex> lock(writeMutex);

	if (sending || writeQueue.empty())
		return;

	sending = true;
	auto payload = writeQueue.front();
	auto self = shared_from_this();

	uint32_t len = htonl(payload->size());
	std::vector<uint8_t> sendBuffer(sizeof(len) + payload->size());
	memcpy(sendBuffer.data(), &len, sizeof(len));
	memcpy(sendBuffer.data() + sizeof(len), payload->data(), payload->size());

	asio::async_write(*ssl_stream, asio::buffer(sendBuffer),
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

//////////////////////////////////////////////////////
// 패킷 유효성 검사
bool Session::isValid(const std::string& packet)
{
	if (packet.empty()) return false;
	const uint8_t* data = reinterpret_cast<const uint8_t*>(packet.data());
	uint8_t opcode = data[0];

	switch (opcode)
	{
	case 0x01: // LOGIN
		return validateLogin(data + 1, packet.size() - 1);
	case 0x02: // MOVE
		if (isAuth)
			return validateMove(data + 1, packet.size() - 1);
		else return false;
	case 0x03: // ATTACK
		if (isAuth)
			return validateAttack(data + 1, packet.size() - 1);
		else return false;
	default:
		std::cout << "Unknown opcode: " << opcode << std::endl;
		return false;
	}
	
}
//////////////////////////////////////////////////////

void Session::Close()
{
	SessionManager::GetInstance().DelSession(id);
	std::cout << "[Session::Close] Session Close, Current UserCount : " << std::to_string(SessionManager::GetInstance().UserCount()) << std::endl;
	// spdlog::info(to_utf8(std::format("[Session::Close] SESSION ID : {} , CurUserCount : {}", std::to_string(id), std::to_string(SessionManager::GetInstance().UserCount()))));
	std::error_code ec;
	ssl_stream->lowest_layer().cancel();
	ssl_stream->shutdown(ec);
}

// username token
bool Session::validateLogin(const uint8_t* data, size_t size)
{
	if (size < 2) return false;
	uint8_t tokenLen = data[0];
	if (size < 1 + tokenLen) return false;

	std::string msg(reinterpret_cast<const char*>(data), size);

	std::istringstream iss(msg);
	std::string userid, token;

	if (!(iss >> userid >> token)) {
		spdlog::warn("validateLogin: 파싱 실패");
		return false;
	}

	std::string key = "user:state:" + userid;
	auto cached = RedisManager::GetInstance().Get(key);
	if (!cached.has_value())
	{
		Close();
		return false;
	}
	std::string savedToken;
	try {
		json j = json::parse(cached.value());

		if (j.contains("Token")) {
			savedToken = j["Token"].get<std::string>();
		}
	}
	catch (const std::exception& e) {
		spdlog::error("JSON 파싱 에러: {}", e.what());
	}

	if (token == savedToken)
	{
		std::cout << "Login 성공: {}" << userid << std::endl;
		isAuth = true;
		return true;
	}

	std::cout << "token not apply : " << token << ":" << savedToken << std::endl;
	Close();
	return false;
}
bool Session::validateMove(const uint8_t* data, size_t size)
{
	return true;
}
bool Session::validateAttack(const uint8_t* data, size_t size)
{
	return true;
}