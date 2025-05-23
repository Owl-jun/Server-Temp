#pragma once
class Session;

class SessionManager {
private:
	std::unordered_map<int, std::weak_ptr<Session>> Sessions;
	std::vector<std::string> validPacketIDs;

	SessionManager();
	~SessionManager() = default;
	SessionManager(const SessionManager&) = delete;
	SessionManager& operator=(const SessionManager&) = delete;

public:
	static SessionManager& GetInstance() {
		static SessionManager instance;
		return instance;
	}
	static int make_UniqueId() {
		static int g_id = 0;
		return g_id++;
	}

	void AddSession(const int id, const std::shared_ptr<Session>& s);
	void DelSession(int id);
	void BroadCast(std::shared_ptr<std::string> msg);

	//////////////////////////////////////////////////////
	// get, set
	std::vector<std::string>& getValidIds();
	std::unordered_map<int , std::weak_ptr<Session>>& get_Sessions();
	int UserCount();
};
