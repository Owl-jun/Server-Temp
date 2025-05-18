#pragma once
#include <memory>
#include <vector>
#include <algorithm>
#include <string>
#include "Session.h"

class SessionManager {
private:
	std::vector<std::weak_ptr<Session>> Sessions;
	std::vector<std::string> validPacketIDs;
	SessionManager() {
		validPacketIDs = { "CHAT", "MOVE", "LOGIN" };
	}
	~SessionManager() = default;
	SessionManager(const SessionManager&) = delete;
	SessionManager& operator=(const SessionManager&) = delete;

public:
	static SessionManager& GetInstance() {
		static SessionManager instance;
		return instance;
	}
	
	static int getUniqueId() { static int g_id = 0; return g_id++; }
	std::vector<std::string>& getValidIds() { return validPacketIDs; }

	void AddSession(const std::shared_ptr<Session>& s) {
		Sessions.push_back(s);
	}

	void DelSession(int id) {
		Sessions.erase(std::remove_if(Sessions.begin(), Sessions.end(),
			[id](const std::weak_ptr<Session>& weak_s) {
				if (auto s = weak_s.lock()) {
					return s->get_id() == id;
				}
				return true;  // 이미 소멸됨
			}), Sessions.end());
	}
	std::vector<std::weak_ptr<Session>>& get_Sessions() { return Sessions; }

	void BroadCast(std::shared_ptr<std::string> msg) {
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
};
