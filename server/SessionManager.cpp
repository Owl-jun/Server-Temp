#include "pch.h"
#include "SessionManager.hpp"
#include "Session.hpp"

//////////////////////////////////////////////////////
// 패킷 유효성 검사
// 패킷 디자인시, 아래 { } 내 패킷 ID 입력
SessionManager::SessionManager() {
	validPacketIDs = { "TEST" , "CHAT" , "MOVE" , "LOGIN" };
}
//////////////////////////////////////////////////////

void SessionManager::AddSession(const std::shared_ptr<Session>& s) {
	Sessions.push_back(s);
}

void SessionManager::DelSession(int id) {
	Sessions.erase(std::remove_if(Sessions.begin(), Sessions.end(),
		[id](const std::weak_ptr<Session>& weak_s) {
			if (auto s = weak_s.lock()) {
				return s->get_id() == id;
			}
			return true;  // 이미 소멸됨
		}), Sessions.end());
}

void SessionManager::BroadCast(std::shared_ptr<std::string> msg) {
	for (auto it = Sessions.begin(); it != Sessions.end(); ) {
		if (auto session = it->lock()) {
			session->push_WriteQueue(msg);
			++it;
		}
		else {
			it = Sessions.erase(it);
		}
	}
}

//////////////////////////////////////////////////////
// get, set
std::vector<std::string>& SessionManager::getValidIds() { return validPacketIDs; }
std::vector<std::weak_ptr<Session>>& SessionManager::get_Sessions() { return Sessions; }
//////////////////////////////////////////////////////

