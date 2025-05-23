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

void SessionManager::AddSession(const int id, const std::shared_ptr<Session>& s) {
	Sessions[id] = s;
}

void SessionManager::DelSession(int id) {
	Sessions.erase(id);
}

void SessionManager::BroadCast(std::shared_ptr<std::string> msg) {
	for (auto it = Sessions.begin(); it != Sessions.end(); ) {
		if (auto session = it->second.lock()) {
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
std::unordered_map<int , std::weak_ptr<Session>>& SessionManager::get_Sessions() { return Sessions; }
int SessionManager::UserCount() { return Sessions.size(); }
//////////////////////////////////////////////////////

