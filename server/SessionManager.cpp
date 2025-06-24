#include "pch.h"
#include "SessionManager.hpp"
#include "Session.hpp"

SessionManager::SessionManager() {}

void SessionManager::AddSession(const int id, const std::shared_ptr<Session>& s) {
	Sessions[id] = s;
}

void SessionManager::DelSession(int id) {
	Sessions.erase(id);
}

void SessionManager::BroadCast(uint8_t opcode, std::shared_ptr<std::string> msg) {
	for (auto it = Sessions.begin(); it != Sessions.end(); ) {
		if (auto session = it->second.lock()) {
			session->push_WriteQueue(opcode, msg);
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

